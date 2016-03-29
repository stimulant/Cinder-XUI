#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "XScene.h"
#include "XUI.h"
#include "XAssetManager.h"
#include <boost/algorithm/string.hpp>
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"

using namespace ci;
using namespace ci::app; // for TouchEvent and TouchEvent::Touch
using namespace xui;

XSceneRef XScene::create()
{
    XSceneRef ref = XSceneRef( new XScene() );
    ref->mRoot = XSceneWeakRef( ref );
    return ref;
}

void parseItem(const rapidxml::xml_node<> &node, XmlTree *parent, XmlTree *result, const XmlTree::ParseOptions &options)
{
	*result = XmlTree(node.name(), node.value(), parent);
	for (const rapidxml::xml_node<> *item = node.first_node(); item; item = item->next_sibling()) {
		XmlTree::NodeType type;
		switch (item->type()) {
		case rapidxml::node_element:
			type = XmlTree::NODE_ELEMENT;
			break;
		case rapidxml::node_cdata: {
			if (options.getCollapseCData()) {
				result->setValue(result->getValue() + item->value());
				continue;
			}
			else {
				type = XmlTree::NODE_CDATA;
			}
		}
								   break;
		case rapidxml::node_comment:
			type = XmlTree::NODE_COMMENT;
			break;
		case rapidxml::node_doctype: {
			result->setDocType(item->value());
			continue;
		}
		case rapidxml::node_data: {
			if (!options.getIgnoreDataChildren())
				type = XmlTree::NODE_DATA;
			else
				continue;
		}
								  break;
		default:
			continue;
		}

		result->getChildren().push_back(std::unique_ptr<XmlTree>(new XmlTree));
		parseItem(*item, result, result->getChildren().back().get(), options);
		result->getChildren().back()->setNodeType(type);
	}

	for (rapidxml::xml_attribute<> *attr = node.first_attribute(); attr; attr = attr->next_attribute())
		result->getAttributes().push_back(XmlTree::Attr(result, attr->name(), attr->value()));
}

// create from a Xml string
// NOTE: this create function does not watch an asset or do any exception handling
XSceneRef XScene::createFromXmlString(std::string xmlString)
{
    XUI::getInstance()->consoleOut("Loading XUI Scene");
	XSceneRef ref = XSceneRef(new XScene());
    ref->mRoot = XSceneWeakRef( ref );
	
	// do manual parsing of xml tree so we can catch location
	XmlTree xmlDoc;
	std::string strCopy(xmlString);
	rapidxml::xml_document<> doc;    // character type defaults to char

	try 
	{
		doc.parse<rapidxml::parse_doctype_node>(&strCopy[0]);
	}
	catch (rapidxml::parse_error &e)
	{
		long count = e.where<const char>() - (const char *)&strCopy[0];
		throw XUIParseError(e.what(), count);
	}

	parseItem(doc, NULL, &xmlDoc, XmlTree::ParseOptions().collapseCData(false));
	xmlDoc.setNodeType(XmlTree::NodeType::NODE_DOCUMENT); // call this after parse - constructor replaces it	

	// load xml into scene
	ref->loadXml( xmlDoc / "Scene");
    ref->loadLuaCode();
	return ref;
}

XSceneRef XScene::create(std::string xmlAsset)
{
	XUI::getInstance()->consoleOut("Loading XUI Scene");
	XSceneRef ref = XSceneRef( new XScene() );
	ref->loadXmlAsset(xmlAsset);
	return ref;
}

XScene::XScene()
{
    mParent = XNodeRef(); // NULL, we are the parent (crash rather than recurse)
    mRoot = XSceneRef();  // NULL, will be set in create() because we are the root    
}

XScene::~XScene()
{
}

void XScene::update(double elapsedSeconds)
{
	XNode::update(elapsedSeconds);
}

bool XScene::mouseDown( ci::app::MouseEvent event )
{
    event.setPos(transformEventPos(event.getPos()));
	return deepMouseDown( event ); // recurses to children
}

bool XScene::mouseUp( ci::app::MouseEvent event )
{
    event.setPos(transformEventPos(event.getPos()));
	return deepMouseUp( event ); // recurses to children
}

bool XScene::mouseDrag( ci::app::MouseEvent event )
{
    event.setPos(transformEventPos(event.getPos()));
	return deepMouseDrag( event ); // recurses to children
}

bool XScene::touchesBegan( TouchEvent event )
{
    TouchEvent newEvent;
    for (auto& touch : event.getTouches())
    {
        newEvent.getTouches().push_back(
                                        TouchEvent::Touch(transformEventPos(touch.getPos()),
                                                          transformEventPos(touch.getPrevPos()),
                                                          touch.getId(), touch.getTime(), NULL));
    }
    
	// handle guestures
	for (XNodeRef &node : mChildren)
		node->guesturesBeganInternal(newEvent);

	// handle touches
    bool consumed = true;
    for(const TouchEvent::Touch &touch : newEvent.getTouches()) {
        consumed = deepTouchBegan( touch ) && consumed; // recurses to children
    }

    return consumed; // only true if all touches were consumed
}

bool XScene::touchesMoved( TouchEvent event )
{
    TouchEvent newEvent;
    for (auto& touch : event.getTouches())
    {
        newEvent.getTouches().push_back(
                                        TouchEvent::Touch(transformEventPos(touch.getPos()),
                                                          transformEventPos(touch.getPrevPos()),
                                                          touch.getId(), touch.getTime(), NULL));
    }
    
    // handle guestures
	for (XNodeRef &node : mChildren)
		node->guesturesMovedInternal(newEvent);

    bool consumed = true;
    for(const TouchEvent::Touch &touch : newEvent.getTouches()) {
        consumed = deepTouchMoved( touch ) && consumed; // recurses to children
    }
    return consumed; // only true if all touches were consumed
}

bool XScene::touchesEnded( TouchEvent event )
{
    TouchEvent newEvent;
    for (auto& touch : event.getTouches())
    {
        newEvent.getTouches().push_back(
                                        TouchEvent::Touch(transformEventPos(touch.getPos()),
                                                          transformEventPos(touch.getPrevPos()),
                                                          touch.getId(), touch.getTime(), NULL));
    }
    
    // handle guestures
	for (XNodeRef &node : mChildren)
		node->guesturesEndedInternal(newEvent);

    bool consumed = true;
    for(const TouchEvent::Touch &touch : newEvent.getTouches()) {
        consumed = deepTouchEnded( touch ) && consumed; // recurses to children
    }    
    return consumed; // only true if all touches were consumed
}

void XScene::dispatchStateEvent( const std::string& event )
{
	// dispatch to callback
	for( CallbackMgr<void (std::string)>::iterator cbIter = mCbStateEvent.begin(); cbIter != mCbStateEvent.end(); ++cbIter ) {
		(cbIter->second)( event );
    }
}

void XScene::loadXmlAsset(std::string xmlAsset)
{
	XAssetManager::load( xmlAsset, [this](DataSourceRef dataSource){
		try {
			loadXml( XmlTree( dataSource, XmlTree::ParseOptions().collapseCData(false) ) / "Scene" );
		} catch ( ci::XmlTree::Exception &boom ) {
			console() << "XML Tree Error: " << boom.what() << std::endl;
			return;
		} catch (rapidxml::parse_error &e)
		{
			console() << "XML Parse Error: " << e.what() << std::endl;
			return;
		} catch (...) {
			console() << "XML Error!!!" << std::endl;
			return;
		}

		console() << "XML Load Success" << std::endl;
    } );
}

void XScene::loadXml(ci::XmlTree &xml)
{
	XNode::loadXml(xml);
    
    mViewportSize.x = (int)xml.getAttributeValue( "viewportWidth", -1.0f );
	mViewportSize.y = (int)xml.getAttributeValue("viewportHeight", -1.0f);

    std::string modalStr = xml.getAttributeValue<std::string>("isModal", "true");
    mIsModal = (modalStr == "true" || modalStr == "TRUE" || atoi(modalStr.c_str()) == 1);

	// globally bind all nodes to other nodes in the lua global space
	// do this after all nodes in scene have been loaded so that all children exist to be bound
	XScript::LuaGlobalBindAllChildren(this);
}

void XScene::deepDraw(float opacity, vec2 offset)
{
    if (mViewportSize.x != -1.0f && mViewportSize.y != -1.0f)
        gl::setMatricesWindow(mViewportSize);
    
    gl::translate(offset * vec2(mViewportSize));
    XNode::deepDraw(opacity);
}

ivec2 XScene::transformEventPos(ivec2 pos) const
{
    if (mViewportSize.x != -1.0f && mViewportSize.y != -1.0f)
        return ivec2((float)pos.x * (float)mViewportSize.x / (float)getWindowWidth(),
                     (float)pos.y * (float)mViewportSize.y / (float)getWindowHeight());
    else
        return pos;
}