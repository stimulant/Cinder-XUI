#include "XNode.h"
#include "XScene.h"
#include "XScript.h"
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"
#include "cinder/app/AppNative.h"
#include "cinder/app/AppBasic.h"
#include <boost/range/adaptor/reversed.hpp>
#include <boost/algorithm/string.hpp>

// node types
#include "XRect.h"
#include "XImage.h"
#include "XText.h"
#include "XSVG.h"

using boost::make_tuple;

using namespace ci;
using namespace ci::app;
using namespace xui;

int XNode::sNextNodeId = 1;

XNodeState::XNodeState(XNodeRef node, ci::XmlTree &xml)
{
	mNode = node;
	mId = xml.getAttributeValue<std::string>( "id", "" );
	mBase = xml.getAttributeValue<std::string>( "base", "" );
	mEvent = xml.getAttributeValue<std::string>( "event", "" );
	
	for ( auto &xmlChild : xml.getChildren() ) 
	{
		if (xmlChild->isElement() && xmlChild->getTag() == "Property")
		{
			std::string target = xmlChild->getAttributeValue<std::string>( "target", "" );
			std::string type = xmlChild->getAttributeValue<std::string>( "type", "" );
			std::string value = xmlChild->getAttributeValue<std::string>( "value", "" );

			float time = xmlChild->getAttributeValue( "time", 0.0f );

			EaseFn easeFn = EaseNone();
			std::string ease = xmlChild->getAttributeValue<std::string>( "ease", "" );
			if (ease == "None")					easeFn = EaseNone();
			if (ease == "InQuad")				easeFn = EaseInQuad();
			if (ease == "OutQuad")				easeFn = EaseOutQuad();
			if (ease == "InCubic")				easeFn = EaseInCubic();
			if (ease == "OutCubic")				easeFn = EaseOutCubic();
			if (ease == "InQuart")				easeFn = EaseInQuart();
			if (ease == "OutQuart")				easeFn = EaseOutQuart();
			if (ease == "InOutQuart")			easeFn = EaseInOutQuart();
			if (ease == "InQuint")				easeFn = EaseInQuint();
			if (ease == "OutQuint")				easeFn = EaseOutQuint();
			if (ease == "InOutQuint")			easeFn = EaseInOutQuint();
			if (ease == "InExpo")				easeFn = EaseInExpo();
			if (ease == "OutExpo")				easeFn = EaseOutExpo();
			if (ease == "InCirc")				easeFn = EaseInCirc();
			if (ease == "OutCirc")				easeFn = EaseOutCirc();
			if (ease == "InOutCirc")			easeFn = EaseInOutCirc();
			if (ease == "InSine")				easeFn = EaseInSine();
			if (ease == "OutSine")				easeFn = EaseOutSine();
			if (ease == "InOutSine")			easeFn = EaseInOutSine();
			if (ease == "InBack")				easeFn = EaseInBack();
			if (ease == "OutBack")				easeFn = EaseOutBack();
			if (ease == "InOutBack")			easeFn = EaseInOutBack();
			if (ease == "InBounce")				easeFn = EaseInBounce();
			if (ease == "OutBounce")			easeFn = EaseOutBounce();
			if (ease == "InOutBounce")			easeFn = EaseInOutBounce();

			mProperties.push_back( XNodeStateProperty( target, type, value, time, easeFn ) );
		}
	}
}

void XNodeState::set()
{
	// set our base state if we have one first (this may recurse)
	if (mBase != "")
		mNode->setState( mBase );

	for ( auto &prop : mProperties ) 
	{
		XNodeRef node = mNode;

		// if we have a target, set the property of the target node instead
		if ( prop.mTarget != "" )
			node = mNode->getChildById( prop.mTarget );

		if ( node )
			node->setProperty( prop );
	}

	// also send any event associated with this state (via our parent Scene's callback
	if ( mEvent != "" )
		mNode->dispatchStateEvent( mEvent );
}

XNode::XNode() : mVisible(true), mX(0.0f), mY(0.0f), mScale(ci::Vec2f(1.0f, 1.0f)), mRotation(0.0f), mOpacity(1.0f), mScript(NULL)
{	
}

XNodeRef XNode::create()
{
    return XNodeRef( new XNode() );
}

XNodeRef XNode::create( ci::XmlTree &xml )
{
	XNodeRef ref = XNodeRef( new XNode() );
	ref->loadXml(xml);
	return ref;
}

void XNode::loadXml(ci::XmlTree &xml)
{
	removeChildren();

	// get/set id if we have one, otherwise auto-generate
	if (xml.hasAttribute("id"))
	{
		mId = xml.getAttributeValue<std::string>("id");
	}
	else
	{
		std::stringstream ss;
		ss << "node_" << sNextNodeId++;
		mId = ss.str();
	}

	// get/set other properties
	Vec2f pos;

	std::string xStr = xml.getAttributeValue<std::string>( "x", "" );
	xStr.erase(std::remove(xStr.begin(), xStr.end(), 'p'), xStr.end()); xStr.erase(std::remove(xStr.begin(), xStr.end(), 'x'), xStr.end());
	try { pos.x = fromString<float>(xStr); } catch(...) { pos.x = 0.0f; }

	std::string yStr = xml.getAttributeValue<std::string>( "y", "" );
	yStr.erase(std::remove(yStr.begin(), yStr.end(), 'p'), yStr.end()); yStr.erase(std::remove(yStr.begin(), yStr.end(), 'x'), yStr.end());
	try { pos.y = fromString<float>(yStr); } catch(...) { pos.y = 0.0f; }

	setPos( pos );

	float opacity = xml.getAttributeValue( "opacity", 1.0f );
	setOpacity( opacity );

	float rotation = xml.getAttributeValue( "rotation", 0.0f );
	setRotation( rotation );

	// load children
	std::string luaCode = "";
	for ( auto &xmlChild : xml.getChildren() ) 
	{
		// handle cdata (lua script)
		if (xmlChild->isCData())
		{
			luaCode = xmlChild->getValue();
		}

		// handle/create children
		else if (xmlChild->isElement())
		{
			if (boost::iequals(xmlChild->getTag(), "Rect"))
				addChild(XRect::create(*xmlChild));
			else if (boost::iequals(xmlChild->getTag(), "Image"))
				addChild(XImage::create(*xmlChild));
			else if (boost::iequals(xmlChild->getTag(), "Svg"))
				addChild(XSVG::create(*xmlChild));
			else if (boost::iequals(xmlChild->getTag(), "Text"))
				addChild(XText::create(*xmlChild));

			// special case for states
			else if (xmlChild->getTag() == "State" && xmlChild->hasAttribute("id"))
			{
				mStates[xmlChild->getAttributeValue<std::string>("id")] = XNodeState(XNodeRef(shared_from_this()), *xmlChild);
			}
		}
	}

	// make sure this is after loading children so that we can bind children as well
	if (luaCode != "")
	{
		mScript = new XScript(); 
		mScript->bindNode(this);
		mScript->loadString(luaCode);
	}
}

void XNode::addChild( XNodeRef child )
{
    mChildren.push_back( child );
    child->mParent = XNodeWeakRef( shared_from_this() );
    // if we have a root node, pass it on
    if ( XSceneRef root = mRoot.lock() ) {
        child->deepSetRoot( root );
    }
    // otherwise just wait, it will be set eventually
}

void XNode::addChildAt( XNodeRef child, const int &index )
{
    mChildren.insert( mChildren.begin() + index, child );
    child->mParent = XNodeWeakRef( shared_from_this() );
    // if we have a root node, pass it on
    if ( XSceneRef root = mRoot.lock() ) {
        child->deepSetRoot( root );
    }
    // otherwise just wait, it will be set eventually
}

void XNode::setChildIndex( XNodeRef child, const int &index )
{
    std::vector<XNodeRef>::iterator i = std::find(mChildren.begin(), mChildren.end(), child);
    if ( i != mChildren.end() ) {
        mChildren.erase( i );
        mChildren.insert( mChildren.begin() + index, child );
    }
    else {
        addChildAt( child, index );
    }
}

int XNode::getChildIndex( XNodeRef child )
{
    std::vector<XNodeRef>::iterator i = std::find(mChildren.begin(), mChildren.end(), child);
    int pos = i - mChildren.begin();
    if (pos < (int)mChildren.size()) {
        return pos;
    }
    return -1;
}

void XNode::removeChild( XNodeRef child )
{
    std::vector<XNodeRef>::iterator i = std::find(mChildren.begin(), mChildren.end(), child);
    if ( i != mChildren.end() ) {
        mChildren.erase( i );
        child->mParent = XNodeWeakRef();
        child->deepSetRoot( XSceneRef() ); // clears root for all children
    }
}

XNodeRef XNode::removeChildAt( const int &index )
{
    if (index < (int)mChildren.size()) {
        std::vector<XNodeRef>::iterator i = mChildren.begin() + index;
        XNodeRef child = *i;
        mChildren.erase( i );
        child->mParent = XNodeRef();
        child->deepSetRoot( XSceneRef() );
        return child;
    }
    return XNodeRef();
}

void XNode::removeChildren()
{
    int numChildren = getNumChildren();
    for (int i = numChildren - 1; i >= 0; i--) {
        removeChildAt(i);
    }    
}

XNodeRef XNode::getChildById( const std::string &childId ) const
{
    for (std::vector<XNodeRef>::const_iterator i = mChildren.begin(); i != mChildren.end(); ++i) {
        if ( (*i)->getId() == childId ) {
            return *i;
        }
    }
    return XNodeRef(); // aka NULL
}

void XNode::setState( std::string stateId )
{
	std::map<std::string, XNodeState>::iterator it = mStates.find( stateId );
	if ( it != mStates.end() ) 
	{
		it->second.set();
	}
}

void XNode::setProperty( const XNodeStateProperty& prop )
{
	if (prop.mType == "x")
		app::timeline().apply( &mX, (float)atof(prop.mValue.c_str()), prop.mTime, prop.mEaseFn );
	else if (prop.mType == "y")
		app::timeline().apply( &mY, (float)atof(prop.mValue.c_str()), prop.mTime, prop.mEaseFn );
	else if (prop.mType == "opacity")
		app::timeline().apply( &mOpacity, (float)atof(prop.mValue.c_str()), prop.mTime, prop.mEaseFn );
}

void XNode::test()
{
	app::console() << "test" << std::endl;
}

void XNode::deepUpdate( double elapsedSeconds )
{
    if (mVisible) {

        // update self
        update( elapsedSeconds );

		// lua update
		if (mScript)
			mScript->call( "update", elapsedSeconds );

        // update children
        for(XNodeRef &child : mChildren) {        
            child->deepUpdate( elapsedSeconds );
        }
    }
}

void XNode::deepDraw(float opacity)
{    
    if (mVisible) {
		gl::pushModelView();

		// create and settransform
		ci::Matrix44f transform;
		transform *= ci::Matrix44f::createTranslation( ci::Vec3f( mX, mY, 0.0f ) );
		transform *= ci::Matrix44f::createRotation( ci::Vec3f( 0.0f, 0.0f, mRotation ) );
		transform *= ci::Matrix44f::createScale( ci::Vec3f( mScale, 1.0f ) );
		gl::multModelView( transform );

		gl::enableAlphaBlending();

        // draw self    
        draw(opacity);
        
        // draw children
        for(XNodeRef &child : mChildren) {        
            child->deepDraw(opacity * mOpacity);
        }

		gl::color( ColorA::white() );
		gl::popModelView();
    }     
}

Matrix44f XNode::getConcatenatedTransform() const
{
	Matrix44f transform = mTransform;
	transform *= Matrix44f::createTranslation( Vec3f( mX, mY, 0.0f ) );
	transform *= Matrix44f::createRotation( Vec3f( mRotation, 0.0f, 0.0f ) );
	transform *= Matrix44f::createScale( Vec3f( mScale, 1.0f ) );

    if ( XNodeRef parent = mParent.lock() ) {
        return parent->getConcatenatedTransform() * transform;
    }
    return transform;
}

Vec2f XNode::localToGlobal( const Vec2f &pos )
{
    Vec3f globalPos = (getConcatenatedTransform() * Vec3f( pos.x, pos.y, 0));
    return (globalPos).xy();
}

Vec2f XNode::globalToLocal( const Vec2f &pos )
{
	Matrix44f concatTransform = getConcatenatedTransform();
    Matrix44f invMtx = concatTransform.inverted();
    return (invMtx * Vec3f(pos.x,pos.y,0)).xy();    
}

bool XNode::deepTouchBegan( TouchEvent::Touch touch )
{
    if (!mVisible) {
        return false;
    }
    bool consumed = false;
    // check children
    // use reverse so that things that will be drawn on top are checked first
    for (XNodeRef &node : boost::adaptors::reverse(mChildren)) {
        if (node->deepTouchBegan(touch)) {
            consumed = true;
            mActiveTouches[touch.getId()] = node;
            break; // first child wins (touch can't affect more than one child node)
        }
    }    
    if (!consumed) {
        // check self
        if (touchBegan(touch)) {
            XNodeRef thisRef = shared_from_this();
            mActiveTouches[touch.getId()] = thisRef;
            dispatchTouchBegan( XSceneEventRef( new XSceneEvent( thisRef, thisRef, touch ) ) );
            consumed = true;
        }
    }
    return consumed;
}

bool XNode::deepTouchMoved( TouchEvent::Touch touch )
{
    if (!mVisible) {
        return false;
    }    
    // in this current implementation, children only receive touchMoved calls 
    // if they returned true for the touch with the same ID in touchesBegan
    bool consumed = false;
    if ( mActiveTouches.find(touch.getId()) != mActiveTouches.end() ) {
        XNodeRef node = mActiveTouches[touch.getId()];
        if (node->getId() == this->getId()) {
            // check self
            consumed = touchMoved(touch);
            if (consumed) {
                dispatchTouchMoved( XSceneEventRef( new XSceneEvent( node, node, touch ) ) );
            }
        }
        else {
            consumed = node->deepTouchMoved( touch );
        }
    }
    return consumed;
}

bool XNode::deepTouchEnded( TouchEvent::Touch touch )
{
    if (!mVisible) {
        return false;
    }    
    // in this current implementation, children only receive touchEnded calls 
    // if they returned true for the touch with the same ID in touchesBegan
    bool consumed = false;
    if ( mActiveTouches.find(touch.getId()) != mActiveTouches.end() ) {
        XNodeRef node = mActiveTouches[touch.getId()];
        if (node->getId() == this->getId()) {
            // check self
            consumed = touchEnded(touch);
            if (consumed) {
                dispatchTouchEnded( XSceneEventRef( new XSceneEvent( node, node, touch ) ) );
            }
        }
        else {
            consumed = node->deepTouchEnded( touch );
        }
        mActiveTouches.erase(touch.getId());
    }
    return consumed;
}

bool XNode::deepHitTest( const Vec2f &screenPos )
{
    if (mVisible) {
        // test children
        for(XNodeRef &child : mChildren) {
            if ( child->deepHitTest( screenPos ) ) {
                return true;
            }
        }
        // test self
        return this->hitTest( screenPos );
    }
    return false;
}

void XNode::deepSetRoot( XSceneRef root )
{
    for(XNodeRef &child : mChildren) {        
        // propagate to children first
        child->deepSetRoot( root );
    }
    // and then set self
    if (root) {
        mRoot = root;
        addedToScene(); // notify subclasses that mRoot and mParent are now valid     
    } else {
        removedFromScene(); // notify subclasses that mRoot and mParent are about to be invalid     
        mRoot = XSceneWeakRef();
    }    
}

void XNode::dispatchStateEvent( const std::string& event )
{
	// dispatch to our parent
	if (XNodeRef parent = getParent()) {
		parent->dispatchStateEvent( event );
    }
}

void XNode::dispatchTouchBegan( XSceneEventRef eventRef )
{ 
	setState("pressed");

	bool handled = false;
	for( CallbackMgr<bool (XSceneEventRef)>::iterator cbIter = mCbTouchBegan.begin(); ( cbIter != mCbTouchBegan.end() ) && ( ! handled ); ++cbIter ) {
		handled = (cbIter->second)( eventRef );
    }
	if( !handled )	{
        if (XNodeRef parent = getParent()) {
            eventRef->setSourceRef( parent );
            parent->dispatchTouchBegan( eventRef );
        }        
    }    
}

void XNode::dispatchTouchMoved( XSceneEventRef eventRef )
{ 
	setState("moved");

	bool handled = false;
	for( CallbackMgr<bool (XSceneEventRef)>::iterator cbIter = mCbTouchMoved.begin(); ( cbIter != mCbTouchMoved.end() ) && ( ! handled ); ++cbIter ) {
		handled = (cbIter->second)( eventRef );
    }
	if( !handled )	{
        if (XNodeRef parent = getParent()) {
            eventRef->setSourceRef( parent );    
            parent->dispatchTouchMoved( eventRef );
        }        
    }    
}

void XNode::dispatchTouchEnded( XSceneEventRef eventRef )
{ 
	setState("released");

	bool handled = false;
	for( CallbackMgr<bool (XSceneEventRef)>::iterator cbIter = mCbTouchEnded.begin(); ( cbIter != mCbTouchEnded.end() ) && ( ! handled ); ++cbIter ) {
		handled = (cbIter->second)( eventRef );
    }
	if( !handled )	{
        if (XNodeRef parent = getParent()) {
            eventRef->setSourceRef( parent );
            parent->dispatchTouchEnded( eventRef );
        }        
    }
}

ci::ColorA xui::hexToColor( const std::string &hex )
{
	std::stringstream converter(hex);
	unsigned int value;
	converter >> std::hex >> value;

	float a = ((value >> 24) & 0xFF) / 255.0f;
	float r = ((value >> 16) & 0xFF) / 255.0f;
	float g = ((value >> 8) & 0xFF) / 255.0f;
	float b = ((value) & 0xFF) / 255.0f;

	return ci::ColorA(r, g, b, a);
}