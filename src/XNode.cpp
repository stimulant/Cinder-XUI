#include "XNode.h"
#include "XScene.h"
#include "XScript.h"
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"
#include <boost/range/adaptor/reversed.hpp>
#include <boost/algorithm/string.hpp>

// node types
#include "XRect.h"
#include "XImage.h"
#include "XButton.h"
#include "XMovie.h"
#include "XText.h"
#if defined(CINDER_AWESOMIUM)
#include "XWeb.h"
#endif
#include "XSVG.h"
#include "XCarousel.h"
#include "XControl.h"

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
            if (ease == "InOutQuad")			easeFn = EaseInOutQuad();
			if (ease == "InCubic")				easeFn = EaseInCubic();
			if (ease == "OutCubic")				easeFn = EaseOutCubic();
            if (ease == "InOutCubic")			easeFn = EaseInOutCubic();
			if (ease == "InQuart")				easeFn = EaseInQuart();
			if (ease == "OutQuart")				easeFn = EaseOutQuart();
			if (ease == "InOutQuart")			easeFn = EaseInOutQuart();
			if (ease == "InQuint")				easeFn = EaseInQuint();
			if (ease == "OutQuint")				easeFn = EaseOutQuint();
			if (ease == "InOutQuint")			easeFn = EaseInOutQuint();
			if (ease == "InExpo")				easeFn = EaseInExpo();
			if (ease == "OutExpo")				easeFn = EaseOutExpo();
            if (ease == "InOutExpo")			easeFn = EaseInOutExpo();
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

XNode::XNode() : mVisible(true), mEnabled(true), mX(0.0f), mY(0.0f), mScale(ci::vec2(1.0f, 1.0f)), mRotation(0.0f), mOpacity(1.0f), mScript(NULL), mMouseDownInside(false)
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
	vec2 pos;
	pos.x = xml.getAttributeValue("x", 0.0f);
	pos.y = xml.getAttributeValue("y", 0.0f);
	setPos( pos );

	float opacity = xml.getAttributeValue( "opacity", 1.0f );
	setOpacity( opacity );

	float rotation = xml.getAttributeValue( "rotation", 0.0f );
	setRotation( rotation );

	std::string enabledStr = xml.getAttributeValue<std::string>("enabled", "true");
	setEnabled(enabledStr == "true" || enabledStr == "TRUE" || atoi(enabledStr.c_str()) == 1);

	std::string visibleStr = xml.getAttributeValue<std::string>("visible", "true");
	setVisible(visibleStr == "true" || visibleStr == "TRUE" || atoi(visibleStr.c_str()) == 1);

	std::string maskType = xml.getAttributeValue<std::string>( "mask", "none" );
	setMask(maskType);

	// load children
	mLuaCode = "";
	for ( auto &xmlChild : xml.getChildren() ) 
	{
		// handle cdata (lua script)
		if (xmlChild->isCData())
		{
			mLuaCode = xmlChild->getValue();
		}

		// handle/create children
		else if (xmlChild->isElement())
		{
			if (boost::iequals(xmlChild->getTag(), "Rect"))
				addChild(XRect::create(*xmlChild));
			else if (boost::iequals(xmlChild->getTag(), "Image"))
				addChild(XImage::create(*xmlChild));
			else if (boost::iequals(xmlChild->getTag(), "Button"))
				addChild(XButton::create(*xmlChild));
			else if (boost::iequals(xmlChild->getTag(), "Movie"))
				addChild(XMovie::create(*xmlChild));
			else if (boost::iequals(xmlChild->getTag(), "Svg"))
				addChild(XSVG::create(*xmlChild));
			else if (boost::iequals(xmlChild->getTag(), "Text"))
				addChild(XText::create(*xmlChild));
#if defined(CINDER_AWESOMIUM)
			else if (boost::iequals(xmlChild->getTag(), "Web"))
				addChild(XWeb::create(*xmlChild));
#endif
			else if (boost::iequals(xmlChild->getTag(), "Carousel"))
				addChild(XCarousel::create(*xmlChild));
			else if (boost::iequals(xmlChild->getTag(), "Control"))
                addChild(XControl::create(*xmlChild));

			// special case for states
			else if (xmlChild->getTag() == "State" && xmlChild->hasAttribute("id"))
			{
				mStates[xmlChild->getAttributeValue<std::string>("id")] = XNodeState(XNodeRef(shared_from_this()), *xmlChild);
			}
		}
	}
    
    // create script node (don't load lua code yet, wait until we have loaded properties)
    mScript = new XScript();
    mScript->bindNode(this);
}

void XNode::loadLuaCode()
{
    for(XNodeRef &child : mChildren)
        child->loadLuaCode();
    
	if (mLuaCode != "")
		mScript->loadString(mLuaCode);
}

void XNode::recursiveLuaMapChildren(XNode* node)
{
	for (int i = 0; i < node->getNumChildren(); i++)
	{
		if (node->getChildAt(i)->getId() != "")
		{
			XNode* childNode = &(*(node->getChildAt(i)));
			luabridge::push(mScript->getState(), childNode);
			lua_setglobal(mScript->getState(), childNode->getId().c_str());
			recursiveLuaMapChildren(childNode);
		}
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
		mChildren[i]->removeChildren();
        removeChildAt(i);
    }    
}

XNodeRef XNode::getChildById( const std::string &childId ) const
{
    for (std::vector<XNodeRef>::const_iterator i = mChildren.begin(); i != mChildren.end(); ++i) {
        if ( (*i)->getId() == childId )
            return *i;
		else
		{
			XNodeRef foundChild = (*i)->getChildById( childId );
			if (foundChild)
				return foundChild;
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
		app::timeline().appendTo(&mX, (float)atof(prop.mValue.c_str()), prop.mTime, prop.mEaseFn);
	else if (prop.mType == "y")
		app::timeline().appendTo(&mY, (float)atof(prop.mValue.c_str()), prop.mTime, prop.mEaseFn);
	else if (prop.mType == "opacity")
		app::timeline().appendTo(&mOpacity, (float)atof(prop.mValue.c_str()), prop.mTime, prop.mEaseFn);
	else if (prop.mType == "enabled")
		setEnabled(atoi(prop.mValue.c_str()) == 1 || prop.mValue.c_str() == "true" || prop.mValue.c_str() == "TRUE");
	else if (prop.mType == "visible")
		setVisible(atoi(prop.mValue.c_str()) == 1 || prop.mValue.c_str() == "true" || prop.mValue.c_str() == "TRUE");
}

void XNode::deepUpdate( double elapsedSeconds )
{
    if (mVisible) {

        // update self
        update( elapsedSeconds );

		// update guestures
		guesturesUpdate();

		// lua update
		if (mScript)
			mScript->call( "update", elapsedSeconds );

        // update children
        for(XNodeRef &child : mChildren) {        
            child->deepUpdate( elapsedSeconds );
        }
    }
}

void XNode::deepDraw(float opacity, glm::vec2 offset)
{    
    if (mVisible) {
		gl::pushModelView();

		// create and settransform
		ci::mat4 transform;
		//transform *= glm::translate( ci::vec3( mX, mY, 0.0f ) );
		//transform *= glm::rotate( ci::vec3( 0.0f, 0.0f, mRotation );
		//transform *= glm::scale( ci::vec3( mScale, 1.0f ) );
		//gl::multModelMatrix( transform );
		//gl::multModelView( transform );
		gl::translate(mX, mY);
		gl::rotate(mRotation);
		gl::scale(mScale);

		// apply guesture transformation
		gl::multModelMatrix(mGuesturesMatrix);

		gl::enableAlphaBlending();

		// masking via stencil buffer
		// not this only works if you turn on the stencil buffer in Cinder
		switch (mMaskType)
		{
			case MaskNone:
				glDisable( GL_STENCIL_TEST );
				glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE ); glDepthMask(GL_TRUE);
				break;
			case MaskClear:
				glEnable( GL_STENCIL_TEST );
				glClear( GL_STENCIL_BUFFER_BIT );
				break;
			case MaskWrite:
				glEnable( GL_STENCIL_TEST );
				glStencilMask(0x1);
				glStencilFunc(GL_ALWAYS, 0x1, 0x1);
				glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
				glDepthMask(GL_FALSE);
				break;
			case MaskNotEqual:
				glEnable( GL_STENCIL_TEST );
				glStencilMask(0x1);
				glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDepthMask(GL_TRUE);
				break;
			case MaskEqual:
				glEnable( GL_STENCIL_TEST );
				glStencilFunc(GL_EQUAL, 0x1, 0x1);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDepthMask(GL_TRUE);
				break;
		}

		// setup drawing from lua
		if (mScript)
			mScript->call( "startDraw" );

        // draw self    
        draw(opacity);
        
		// finish drawing from lua
		if (mScript)
			mScript->call( "endDraw" );
        
        // draw children
        for(XNodeRef &child : mChildren) {        
            child->deepDraw(opacity * mOpacity);
        }

		gl::color( ColorA::white() );
		gl::popModelView();

		// turn off stenciling
		glDisable( GL_STENCIL_TEST );
		glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE ); glDepthMask(GL_TRUE);
    }     
}

mat4 XNode::getConcatenatedTransform() const
{
	mat4 transform = mTransform;
	
	transform *= glm::translate(vec3(mX, mY, 0.0f));
	transform *= glm::rotate(mRotation, vec3(0.0f, 0.0f, 1.0f));
	transform *= glm::scale(vec3(mScale, 1.0f));
	transform *= mGuesturesMatrix;

    if ( XNodeRef parent = mParent.lock() ) {
        return parent->getConcatenatedTransform() * transform;
    }
    return transform;
}

void XNode::setMask( std::string maskType )
{
	if (maskType == "none")
		mMaskType = MaskNone;
	if (maskType == "clear")
		mMaskType = MaskClear;
	if (maskType == "write")
		mMaskType = MaskWrite;
	if (maskType == "notequal")
		mMaskType = MaskNotEqual;
	if (maskType == "equal")
		mMaskType = MaskEqual;
}

std::string XNode::getMask() const
{
	if (mMaskType == MaskNone)
		return "none";
	if (mMaskType == MaskClear)
		return "clear";
	if (mMaskType == MaskWrite)
		return "write";
	if (mMaskType == MaskNotEqual)
		return "notequal";
	if (mMaskType == MaskEqual)
		return "equal";
	return "";
}

vec2 XNode::localToGlobal( const vec2 &pos )
{
    vec4 globalPos = (getConcatenatedTransform() * vec4( pos.x, pos.y, 0, 1));
	return vec2(globalPos.x, globalPos.y);;
}

vec2 XNode::globalToLocal( const vec2 &pos )
{
	mat4 concatTransform = getConcatenatedTransform();
    mat4 invMtx = glm::inverse(concatTransform);
	vec4 v = invMtx * vec4(pos.x, pos.y, 0, 1);
	return vec2(v.x, v.y);
}


bool XNode::deepMouseDown( ci::app::MouseEvent event )
{
	if (!mVisible || !mEnabled)
        return false;

    bool consumed = false;

    // check children
    // use reverse so that things that will be drawn on top are checked first
    for (XNodeRef &node : boost::adaptors::reverse(mChildren)) 
	{
        if (node->deepMouseDown(event)) 
		{
            //consumed = true;
            break; // first child wins (mouse can't affect more than one child node)
        }
    }    
    if (!consumed) 
	{
        // check self
        if (mouseDownInternal(event)) 
		{
            XNodeRef thisRef = shared_from_this();
            dispatchMouseDown( XSceneEventRef( new XSceneEvent( thisRef, thisRef, event ) ) );
			mMouseDownInside = true;
            //consumed = true;
        }
    }
    return consumed;
}

bool XNode::deepMouseUp( ci::app::MouseEvent event )
{
	if (!mVisible || !mEnabled)
        return false;

    // in this current implementation, children only receive mouseUp calls 
    // if they returned true for mouse inside
    bool consumed = false;
	for (XNodeRef &node : boost::adaptors::reverse(mChildren)) 
	{
		if (node->deepMouseUp(event)) 
		{
			//consumed = true;
			break;
		}
	}

	if (!consumed)
	{
		if (mMouseDownInside)
		{
			// check self
			//consumed = mouseUpInternal(event);
            consumed = mouseUpInternal(event);
			if (consumed) 
			{
				XNodeRef thisRef = shared_from_this();
				dispatchMouseUp( XSceneEventRef( new XSceneEvent( thisRef, thisRef, event ) ) );
			}
			mMouseDownInside = false;
		}
	}
    
    return consumed;
}

bool XNode::deepMouseDrag( ci::app::MouseEvent event )
{
	if (!mVisible || !mEnabled)
        return false;

    // in this current implementation, children only receive mouseDrag calls 
    // if they have mouse down inside
    bool consumed = false;
	for (XNodeRef &node : boost::adaptors::reverse(mChildren)) 
	{
		if (node->deepMouseDrag(event)) 
		{
			consumed = true;
			break;
		}
	}

	if (!consumed && mMouseDownInside)
	{
		// check self
		consumed = mouseDragInternal(event);
		if (consumed)
		{
			XNodeRef thisRef = shared_from_this();
			dispatchMouseDrag( XSceneEventRef( new XSceneEvent( thisRef, thisRef, event ) ) );
		}
	}
    return consumed;
}


bool XNode::deepTouchBegan( TouchEvent::Touch touch )
{
	if (!mVisible || !mEnabled)
        return false;
    
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
        if (touchBeganInternal(touch)) {
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
	if (!mVisible || !mEnabled)
        return false;

	//app::console() << "id: " << touch.getId() << std::endl;

    // in this current implementation, children only receive touchMoved calls 
    // if they returned true for the touch with the same ID in touchesBegan
    bool consumed = false;
    if ( mActiveTouches.find(touch.getId()) != mActiveTouches.end() ) 
	{
        XNodeRef node = mActiveTouches[touch.getId()];
        if (node->getId() == this->getId()) 
		{
            // check self
            consumed = touchMovedInternal(touch);
            if (consumed)
                dispatchTouchMoved( XSceneEventRef( new XSceneEvent( node, node, touch ) ) );
        }
        else
            consumed = node->deepTouchMoved( touch );
    }
    return consumed;
}

bool XNode::deepTouchEnded( TouchEvent::Touch touch )
{
	if (!mVisible || !mEnabled)
        return false;

    // in this current implementation, children only receive touchEnded calls 
    // if they returned true for the touch with the same ID in touchesBegan
    bool consumed = false;
    if ( mActiveTouches.find(touch.getId()) != mActiveTouches.end() ) 
	{
        XNodeRef node = mActiveTouches[touch.getId()];
        if (node->getId() == this->getId()) 
		{
            // check self
            consumed = touchEndedInternal(touch);
            if (consumed)
                dispatchTouchEnded( XSceneEventRef( new XSceneEvent( node, node, touch ) ) );
        }
        else
            consumed = node->deepTouchEnded( touch );
        mActiveTouches.erase(touch.getId());
    }
    return consumed;
}

bool XNode::deepHitTest( const vec2 &screenPos )
{
	if (mVisible || !mEnabled)
	{
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

void XNode::dispatchMouseDown( XSceneEventRef eventRef )
{ 
	setState("pressed");

	bool handled = false;
	for( CallbackMgr<bool (XSceneEventRef)>::iterator cbIter = mCbMouseDown.begin(); ( cbIter != mCbMouseDown.end() ) && ( ! handled ); ++cbIter ) {
		handled = (cbIter->second)( eventRef );
    }
	if( !handled )	{
        if (XNodeRef parent = getParent()) {
            eventRef->setSourceRef( parent );
            parent->dispatchMouseDown( eventRef );
        }        
    }    
}

void XNode::dispatchMouseDrag( XSceneEventRef eventRef )
{ 
	setState("moved");

	bool handled = false;
	for( CallbackMgr<bool (XSceneEventRef)>::iterator cbIter = mCbMouseDrag.begin(); ( cbIter != mCbMouseDrag.end() ) && ( ! handled ); ++cbIter ) {
		handled = (cbIter->second)( eventRef );
    }
	if( !handled )	{
        if (XNodeRef parent = getParent()) {
            eventRef->setSourceRef( parent );    
            parent->dispatchMouseDrag( eventRef );
        }        
    }    
}

void XNode::dispatchMouseUp( XSceneEventRef eventRef )
{ 
	setState("released");

	bool handled = false;
	for( CallbackMgr<bool (XSceneEventRef)>::iterator cbIter = mCbMouseUp.begin(); ( cbIter != mCbMouseUp.end() ) && ( ! handled ); ++cbIter ) {
		handled = (cbIter->second)( eventRef );
    }
	if( !handled )	{
        if (XNodeRef parent = getParent()) {
            eventRef->setSourceRef( parent );
            parent->dispatchMouseUp( eventRef );
        }        
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

ci::audio::VoiceRef XNode::getAudioVoice( std::string filename )
{
    ci::audio::VoiceRef voice;
    
    // add the audio voice if it does not already exist
    if (mVoiceMap.find(filename) != mVoiceMap.end())
        voice = mVoiceMap.find(filename)->second;
    else
    {
        voice = audio::Voice::create( audio::load( loadAsset( filename ) ) );
        mVoiceMap[filename] = voice;
    }
    
    return voice;
}

void XNode::playSound( std::string filename )
{
    // get the audio voice if it does not already exist
    ci::audio::VoiceRef voice = getAudioVoice(filename);
    if (voice)
        voice->start();
}

void XNode::luaDelay( std::string luaCode, float delay )
{
    // Simple functor to execute some lua code after a period of time
    struct DelayCode
    {
        DelayCode( XNode* node, std::string code ) : mNode( node ), mCode( code ) {}
        void operator()()
        {
            mNode->getScript()->loadString( mCode );
        }
        XNode* mNode;
        std::string mCode;
    };
    
    timeline().add( DelayCode( this, luaCode ), timeline().getCurrentTime() + delay );
}

ci::ColorA xui::hexToColor( const std::string &hex )
{
	std::stringstream converter(hex);
	unsigned int value;
	converter >> std::hex >> value;

    float r=1.0f, g=1.0f, b=1.0f, a=1.0f;
    if (hex.size() >= 10)
    {
        a = ((value >> 24) & 0xFF) / 255.0f;
        r = ((value >> 16) & 0xFF) / 255.0f;
        g = ((value >> 8) & 0xFF) / 255.0f;
        b = ((value) & 0xFF) / 255.0f;
    }
    else
    {
        r = ((value >> 16) & 0xFF) / 255.0f;
        g = ((value >> 8) & 0xFF) / 255.0f;
        b = ((value) & 0xFF) / 255.0f;
    }

	return ci::ColorA(r, g, b, a);
}