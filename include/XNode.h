#pragma once

#include <map>
#include <vector>
#include <tuple>

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include "cinder/Cinder.h"
#include "cinder/Function.h"
#include "cinder/app/TouchEvent.h"
#include "cinder/app/MouseEvent.h"
#include "cinder/Matrix.h"
#include "cinder/Xml.h"
#include "cinder/Color.h"
#include "cinder/Timeline.h"
#include "cinder/audio/Voice.h"
#include "cinder/audio/Source.h"

#include "XScript.h"

namespace xui {

class XState; // for state
class XScene; // for root
class XNode; // for XNodeRef
class XScript; // for XScript

// for sharing ownership:
typedef std::shared_ptr<XScene> XSceneRef;
typedef std::shared_ptr<XNode> XNodeRef;

// for avoiding circular refs:
typedef std::weak_ptr<XScene> XSceneWeakRef;
typedef std::weak_ptr<XNode> XNodeWeakRef;

struct XNodeStateProperty
{
	XNodeStateProperty( const std::string& target, const std::string& type, const std::string& value, float time, ci::EaseFn easeFn) :
		mTarget(target), mType(type), mValue(value), mTime(time), mEaseFn(easeFn) {};

	std::string mTarget;
	std::string mType;
	std::string mValue;
	float mTime;
	ci::EaseFn mEaseFn;
};

class XNodeState
{
public:
	XNodeState() {}
	XNodeState(XNodeRef node, ci::XmlTree &xml);
    void set();
    
private:
	XNodeRef mNode;
    std::string mId;
	std::string mBase;
	std::string mEvent;
	std::vector<XNodeStateProperty> mProperties;
};

class XSceneEvent
{
public:

    XSceneEvent( XNodeRef targetRef, XNodeRef sourceRef, ci::app::TouchEvent::Touch touch ): 
        mTargetRef(targetRef),
        mSourceRef(sourceRef), 
        mTouch(touch) {}

	XSceneEvent( XNodeRef targetRef, XNodeRef sourceRef, ci::app::MouseEvent event ): 
        mTargetRef(targetRef),
        mSourceRef(sourceRef), 
        mMouseEvent(event) {}
    
    ~XSceneEvent() {}

    // the node this event happened to
    XNodeRef getTargetRef() { return mTargetRef; }

    // the node that dispatched this event
    XNodeRef getSourceRef() { return mSourceRef; }

    // the raw screen-space touch that triggered this event
    ci::app::TouchEvent::Touch getTouch() { return mTouch; }    

	// the raw mouse event that triggered this event
    ci::app::MouseEvent getMouseEvent() { return mMouseEvent; }    

    void setTargetRef( XNodeRef targetRef ) { mTargetRef = targetRef; }
    void setSourceRef( XNodeRef sourceRef ) { mSourceRef = sourceRef; }    
    void setTouch( ci::app::TouchEvent::Touch touch ) { mTouch = touch; }    
	void setMouseEvent( ci::app::MouseEvent mouseEvent ) { mMouseEvent = mouseEvent; }    
    
private:
    
    XNodeRef mTargetRef, mSourceRef;
    ci::app::TouchEvent::Touch mTouch;
	ci::app::MouseEvent mMouseEvent;
};

typedef std::shared_ptr<XSceneEvent> XSceneEventRef;

class XNode : public std::enable_shared_from_this<XNode> {

protected:

    // subclasses can call this, but lib users can't create a XNode directly
    XNode();
    
public:

	typedef	enum { NodeTypeNode, NodeTypeRect, NodeTypeText, NodeTypeImage, NodeTypeButton, NodeTypeMovie, NodeTypeSVG, NodeTypeWeb, NodeTypeCarousel, NodeTypeControl } NodeType;
	typedef enum { MaskNone, MaskClear, MaskWrite, MaskNotEqual, MaskEqual } MaskType;
	virtual XNode::NodeType getType() { return NodeTypeNode; }

    static XNodeRef create();
	static XNodeRef create( ci::XmlTree &xml );
	void loadXml(ci::XmlTree &xml);
	void loadLuaCode();
	void recursiveLuaMapChildren(XNode* node);

    // sub-classing is to be expected
    virtual ~XNode() {}
    
// CONTAINER METHODS
    
    void addChild( XNodeRef child );
    void addChildAt( XNodeRef child, const int &index );
    void setChildIndex( XNodeRef child, const int &index );
    int getChildIndex(  XNodeRef child );
    void removeChild( XNodeRef child );
    void removeChildren();
    XNodeRef removeChildAt( const int &index );
    int getNumChildren() const { return mChildren.size(); }
    XNodeRef getChildAt( const int &index ) const { return mChildren[index]; }
    XNodeRef getChildById( const std::string &childId ) const;

// STATES
    void setState( std::string stateId );
	virtual void setProperty( const XNodeStateProperty& prop );

// VISIBILITY
    
    // if you have behaviors to toggle when things are visible or not, override these
    virtual void setVisible( bool visible ) { mVisible = visible; }
    virtual bool getVisible() const { return mVisible; }

	// if you have behaviors to toggle when things are visible or not, override these
	virtual void setEnabled(bool enabled) { mEnabled = enabled; }
	virtual bool getEnabled() const { return mEnabled; }
    
// LOOP METHODS
    
    // subclasses should mess with these, just draw/update yourself (not children)
    // (deepDraw/Update draws children in correct order)
    virtual void update( double elapsedSeconds ) {}
    virtual void draw(float opacity = 1.0f) {}
    
	void test();

// TRANFORM METHODS

	void setX( float x )						{ mX = x; }
	void setY( float y )						{ mY = y; }
	void setPos( const ci::vec2 &pos )			{ mX = pos.x; mY = pos.y; }
	void setRotation( float rotation )			{ mRotation = rotation; }
	void setScale( const ci::vec2 &scale )		{ mScale = scale; }
	void setOpacity( float opacity )			{ mOpacity = opacity; }

	float getX() const							{ return mX; }
	float getY() const							{ return mY; }
	float getRotation() const					{ return mRotation; }
	ci::vec2 getScale() const					{ return mScale; }
	float getOpacity() const					{ return mOpacity; }

	ci::Anim<float>* getXAnim()					{ return &mX; }
	ci::Anim<float>* getYAnim()					{ return &mY; }
	ci::Anim<float>* getOpacityAnim()			{ return &mOpacity; }

	virtual float getWidth() const				{ return 0.0f; }
	virtual void setWidth(float value)			{ }
	virtual float getHeight() const				{ return 0.0f; }
	virtual void setHeight(float value)			{ }

	virtual ci::ColorA getColor() const 		{ return ci::ColorA(); }
	virtual void setColor( const ci::ColorA &color )	{ }
    virtual ci::ColorA getStrokeColor() const   { return ci::ColorA(); }
    virtual void setStrokeColor( const ci::ColorA &color ) { }
    virtual float getCornerRadius() const       { return 0.0f; }
    virtual void setCornerRadius(float value)   { }
	virtual void setMask(std::string maskType);
	virtual std::string getMask() const;

	// guestures
	virtual bool getPanEnabled() const			{ return false; }
	virtual void setPanEnabled(bool panEnabled) { }
	virtual bool getRotateEnabled() const		{ return false; }
	virtual void setRotateEnabled(bool rotateEnabled) { }
	virtual bool getScaleEnabled() const		{ return false; }
	virtual void setScaleEnabled(bool scaleEnabled) { }

	// text
	virtual std::string getText() const					{ return ""; }
	virtual void setText( const std::string& text )		{
        cinder::app::console() << "test" << std::endl;
    }
	virtual int getTextWidth() const			{ return 0; }
	virtual int getTextHeight() const			{ return 0; }

	// movie
	virtual bool getLoop() const { return false; }
	virtual void setLoop(bool loop) { }
	virtual void seekToStart() { }
	virtual void stop() { }
	virtual void play() { }
    
    // play audio
    void playSound( std::string filename );
    
    // execute some lua code after a period of time
    void luaDelay( std::string luaCode, float delay );

	// carousel
	virtual bool getCarouselEnabled() const		{ return false; }
	virtual void setCarouselEnabled(bool carouselEnabled) { }
    
    // override getConcatenatedTransform to change the behavior of these:
    ci::vec2 localToGlobal( const ci::vec2 &pos );
    ci::vec2 globalToLocal( const ci::vec2 &pos );
    
    // if you have extra transforms/rotations that get applied in draw, override this too:
    virtual ci::mat4 getConcatenatedTransform() const;    
    
// ID METHODS
        
    const std::string& getId() const { return mId; }
    void setId( std::string& newId ) { mId = newId; }
    
// HIERARCHY METHODS
    
    XNodeRef getParent() const { return mParent.lock(); }
    XSceneRef getRoot() const { return mRoot.lock(); }
    
    // if you need mRoot to setup, override this:
    virtual bool addedToScene() { return false; }
    // if you attach things to mRoot in setup, override this too:
    virtual bool removedFromScene() { return false; }
    
// INTERACTION HELPERS

	// mouse interactions
    virtual bool mouseDownInternal( ci::app::MouseEvent event ) { return false; }
    virtual bool mouseDragInternal( ci::app::MouseEvent event ) { return false; }
    virtual bool mouseUpInternal( ci::app::MouseEvent event ) { return false; }
            
    // interactions, one touch at a time
    virtual bool touchBeganInternal( ci::app::TouchEvent::Touch touch ) { return false; }
    // touchMoved/Ended will only be called for touch IDs that returned true in touchBegan
    virtual bool touchMovedInternal( ci::app::TouchEvent::Touch touch ) { return false; }
    virtual bool touchEndedInternal( ci::app::TouchEvent::Touch touch ) { return false; }

	// guestures interactions
	virtual void guesturesUpdate() {}
	virtual void guesturesBeganInternal(ci::app::TouchEvent event) { }
	virtual void guesturesMovedInternal(ci::app::TouchEvent event) { }
	virtual void guesturesEndedInternal(ci::app::TouchEvent event) { }
    
    // override deepHitTest as well if you want to skip hit-testing children
    virtual bool hitTest( const ci::vec2 &screenPos ) { return false; }
    
// RECURSIVE METHODS
    
    // recurse to children and call draw()
    // (public so it can be overridden, but generally considered internal)
    virtual void deepDraw(float opacity = 1.0f, glm::vec2 offset = glm::vec2(0, 0));
    
    // recurse to children and call update()
    // (public so it can be overridden, but generally considered internal)
    virtual void deepUpdate( double elapsedSeconds );

	// recurse to children and call mouseDown/mouseUp/mouseDrag
    // (public so they can be overridden, but generally considered internal)
	virtual bool deepMouseDown( ci::app::MouseEvent event );
	virtual bool deepMouseUp( ci::app::MouseEvent event );
	virtual bool deepMouseDrag( ci::app::MouseEvent event );
    
    // recurse to children and call touchBegan/Moved/Ended
    // (public so they can be overridden, but generally considered internal)
    virtual bool deepTouchBegan( ci::app::TouchEvent::Touch touch );
    virtual bool deepTouchMoved( ci::app::TouchEvent::Touch touch );
    virtual bool deepTouchEnded( ci::app::TouchEvent::Touch touch );
    
    // recurse to children and call hitTest
    // override this if you want to skip hitTesting children
    virtual bool deepHitTest( const ci::vec2 &screenPos );
    
    // notify children that mRoot is valid
    virtual void deepSetRoot( XSceneRef root );
    
// EVENT STUFF
    
	template<typename T>
    ci::CallbackId registerMouseDown( T *obj, bool (T::*callback)(XSceneEventRef) )
	{
		return mCbMouseDown.registerCb(std::bind1st(std::mem_fun(callback), obj));
	}    
	template<typename T>
    ci::CallbackId registerMouseDrag( T *obj, bool (T::*callback)(XSceneEventRef) )
	{
		return mCbMouseDrag.registerCb(std::bind1st(std::mem_fun(callback), obj));
	}    
	template<typename T>
    ci::CallbackId registerMouseUp( T *obj, bool (T::*callback)(XSceneEventRef) )
	{
		return mCbMouseUp.registerCb(std::bind1st(std::mem_fun(callback), obj));
	}

	template<typename T>
    ci::CallbackId registerTouchBegan( T *obj, bool (T::*callback)(XSceneEventRef) )
	{
		return mCbTouchBegan.registerCb(std::bind1st(std::mem_fun(callback), obj));
	}    
	template<typename T>
    ci::CallbackId registerTouchMoved( T *obj, bool (T::*callback)(XSceneEventRef) )
	{
		return mCbTouchMoved.registerCb(std::bind1st(std::mem_fun(callback), obj));
	}    
	template<typename T>
    ci::CallbackId registerTouchEnded( T *obj, bool (T::*callback)(XSceneEventRef) )
	{
		return mCbTouchEnded.registerCb(std::bind1st(std::mem_fun(callback), obj));
	}
	
	void unregisterMouseDown( ci::CallbackId cbId ) { mCbMouseDown.unregisterCb( cbId ); }
    void unregisterMouseDrag( ci::CallbackId cbId ) { mCbMouseDrag.unregisterCb( cbId ); }
    void unregisterMouseUp( ci::CallbackId cbId ) { mCbMouseUp.unregisterCb( cbId ); }

    void unregisterTouchBegan( ci::CallbackId cbId ) { mCbTouchBegan.unregisterCb( cbId ); }
    void unregisterTouchMoved( ci::CallbackId cbId ) { mCbTouchMoved.unregisterCb( cbId ); }
    void unregisterTouchEnded( ci::CallbackId cbId ) { mCbTouchEnded.unregisterCb( cbId ); }

	// dispatch state events up to scene
	virtual void dispatchStateEvent( const std::string& event );
    
    // bubbles events up to parents...
	void dispatchMouseDown( XSceneEventRef eventRef );
	void dispatchMouseDrag( XSceneEventRef eventRef );
	void dispatchMouseUp( XSceneEventRef eventRef );
    void dispatchTouchBegan( XSceneEventRef eventRef );
    void dispatchTouchMoved( XSceneEventRef eventRef );
    void dispatchTouchEnded( XSceneEventRef eventRef );
    
// LUA
	XScript* getScript() { return mScript; }
	void luaCall( const std::string& function )
	{
		if (mScript)
			mScript->call( function );
	}
	template<typename T> T luaCall( const std::string& function )
	{
		T result;
		if (mScript)
			result = mScript->call<T>( function );
		return result;
	}
	template<typename Arg> void luaCall( const std::string& function, const Arg& arg )
	{
		if (mScript)
			mScript->call( function, arg );
	}
	template<typename Arg1, typename Arg2> void luaCall( const std::string& function, const Arg1& arg1, const Arg2& arg2 )
	{
		if (mScript)
			mScript->call( function, arg1, arg2 );
	}
	template<typename Arg1, typename Arg2, typename Arg3> void luaCall( const std::string& function, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3 )
	{
		if (mScript)
			mScript->call( function, arg1, arg2, arg3 );
	}
	template<typename Arg1, typename Arg2, typename Arg3, typename Arg4> void luaCall( const std::string& function, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4 )
	{
		if (mScript)
			mScript->call( function, arg1, arg2, arg3, arg4 );
	}
        
protected:
    
    // weakrefs because we don't "own" these, they're just convenient
    // (full shared pointers would be circular references, we'd never clean up)
    XNodeWeakRef mParent;
    XSceneWeakRef mRoot; // aka "stage"

	// lua script
    std::string mLuaCode;
	XScript* mScript;

    // and this is really all we have, everything else is recursion
    std::string mId;
    bool mVisible;
	bool mEnabled;
	bool mMouseDownInside;

	ci::mat4 mTransform;

	// position, opacity, rotation, scale
	ci::Anim<float> mX;
	ci::Anim<float> mY;
	ci::Anim<float> mOpacity;
	ci::vec2 mScale;
	float mRotation;

	// guesture transformation
	glm::mat4	mGuesturesMatrix;

	// masking
	MaskType mMaskType;

    // normal shared_ptrs because we "own" children
    std::vector<XNodeRef> mChildren;
    
    // keep track of interactions claimed by ID
    std::map<uint64_t, XNodeRef> mActiveTouches;

    // for generating IDs:
    static int sNextNodeId;
    
    // audio voices
    std::map<std::string, ci::audio::VoiceRef> mVoiceMap;
    ci::audio::VoiceRef getAudioVoice( std::string filename );
    
    // for event passing
	ci::CallbackMgr<bool(XSceneEventRef)> mCbTouchBegan;
	ci::CallbackMgr<bool(XSceneEventRef)> mCbTouchMoved;
	ci::CallbackMgr<bool(XSceneEventRef)> mCbTouchEnded;
	ci::CallbackMgr<bool(XSceneEventRef)> mCbMouseDown;
	ci::CallbackMgr<bool(XSceneEventRef)> mCbMouseDrag;
	ci::CallbackMgr<bool(XSceneEventRef)> mCbMouseUp;

	std::map<std::string, XNodeState> mStates;
};

ci::ColorA hexToColor( const std::string &hex );

}