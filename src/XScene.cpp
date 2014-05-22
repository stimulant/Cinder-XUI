#include "cinder/app/App.h"
#include "XScene.h"
#include "XAssetManager.h"

using namespace ci;
using namespace ci::app; // for TouchEvent and TouchEvent::Touch
using namespace xui;

XSceneRef XScene::create()
{
    XSceneRef ref = XSceneRef( new XScene() );
    ref->mRoot = XSceneWeakRef( ref );
    return ref;
}

XSceneRef XScene::create(std::string xmlAsset)
{
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

bool XScene::touchesBegan( TouchEvent event )
{
    bool consumed = true;
    for(const TouchEvent::Touch &touch : event.getTouches()) {
        consumed = deepTouchBegan( touch ) && consumed; // recurses to children
    }
    return consumed; // only true if all touches were consumed
}

bool XScene::touchesMoved( TouchEvent event )
{
    bool consumed = true;
    for(const TouchEvent::Touch &touch : event.getTouches()) {
        consumed = deepTouchMoved( touch ) && consumed; // recurses to children
    }
    return consumed; // only true if all touches were consumed
}

bool XScene::touchesEnded( TouchEvent event )
{
    bool consumed = true;
    for(const TouchEvent::Touch &touch : event.getTouches()) {
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
			console() << "XML Error: " << boom.what() << std::endl;
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
}