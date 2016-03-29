#pragma once
#include "XNode.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"

namespace xui {

typedef std::shared_ptr<class XRect> XRectRef;

class XRect : public XNode
{
public:
    static XRectRef create();
    static XRectRef create( ci::XmlTree &xml );

	virtual XNode::NodeType getType() { return XNode::NodeTypeRect; }

    void draw(float opacity = 1.0f);

	bool mouseDownInternal( ci::app::MouseEvent event );
	bool mouseDragInternal( ci::app::MouseEvent event );
	bool mouseUpInternal( ci::app::MouseEvent event );
    
    // touchEnded will only be called for touch IDs that returned true in touchBegan
    // so we need to implement both methods (there's also touchMoved which works the same way)
    bool touchBeganInternal( ci::app::TouchEvent::Touch touch );
	bool touchMovedInternal( ci::app::TouchEvent::Touch touch );
    bool touchEndedInternal( ci::app::TouchEvent::Touch touch );

	void guesturesBeganInternal(ci::app::TouchEvent event);
	void guesturesMovedInternal(ci::app::TouchEvent event);
	void guesturesEndedInternal(ci::app::TouchEvent event);

	void loadXml( ci::XmlTree &xml );

	void setProperty( const XNodeStateProperty& prop );
    
    // we'll implement hitTest as a helper for touchBegan/touchEnded
    bool hitTest( const ci::vec2 &screenPos );

	// setters
	virtual void setSize( const ci::vec2 &size )				{ mWidth = size.x; mHeight = size.y; }
    
	virtual ci::ColorA getColor() const 						{ return mColor; }
	virtual void setColor( const ci::ColorA &color )			{ mColor = color; }
    virtual ci::ColorA getStrokeColor() const 					{ return mStrokeColor; }
    virtual void setStrokeColor( const ci::ColorA &color )		{ mStrokeColor = color; }
    
    virtual float getCornerRadius() const						{ return mCornerRadius; }
    virtual void setCornerRadius(float value)					{ mCornerRadius = value; }

	virtual float getWidth() const								{ return mWidth; }
	virtual void setWidth(float value)							{ mWidth = value; }
	virtual float getHeight() const								{ return mHeight; }
	virtual void setHeight(float value)							{ mHeight = value; }

	virtual bool getPanEnabled() const							{ return mPanEnabled; }
	virtual void setPanEnabled(bool panEnabled)					{ mPanEnabled = panEnabled; }
	virtual bool getRotateEnabled() const						{ return mRotationEnabled; }
	virtual void setRotateEnabled(bool rotateEnabled)			{ mRotationEnabled = rotateEnabled; }
	virtual bool getScaleEnabled() const						{ return mScaleEnabled; }
	virtual void setScaleEnabled(bool scaleEnabled)				{ mScaleEnabled = scaleEnabled; }
    
protected:
	XRect();
    
	ci::Anim<float> mWidth;
	ci::Anim<float> mHeight;
	ci::ColorA mColor;
    ci::ColorA mStrokeColor;
    float mCornerRadius;

	// GESTURES
	bool		mTapEnabled;
	bool		mPanEnabled;
	bool		mScaleEnabled;
	bool		mRotationEnabled;

	std::vector<ci::app::TouchEvent::Touch>	mGuestureTouches;
	double		mGuestureTouchDelay;
	double		mGuestureTouchTime;
	float		mInterpolationSpeed;
	ci::vec2	mTapPosition;
	double		mTapTime;
	unsigned int mTapCount;
	float		mTapThreshold;
	double		mTapDelay;

	ci::vec2	mPan;
	ci::vec2	mPanMax;
	ci::vec2	mPanMin;
	ci::vec2	mPanSpeed;
	ci::vec2	mPanTarget;
	ci::vec2	mPanThreshold;

	ci::quat	mRotation;
	float 		mRotationSpeed;
	ci::quat	mRotationTarget;
	float 		mRotationThreshold;

	ci::vec2	mScale;
	ci::vec2	mScaleMax;
	ci::vec2	mScaleMin;
	ci::vec2	mScaleSpeed;
	bool		mScaleSymmetry;
	ci::vec2	mScaleTarget;
	ci::vec2 	mScaleThreshold;

	double		mTouchDelay;
	double		mTouchTime;

	enum : size_t
	{
		MotionType_PanX,
		MotionType_PanY,
		MotionType_Rotation,
		MotionType_ScaleX,
		MotionType_ScaleY
	} typedef MotionType;
	typedef std::pair<MotionType, float> Motion;

	void addGuestureTouch(const ci::app::TouchEvent::Touch& touch);
	void guesturesUpdate();
};

}