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
    
    // touchEnded will only be called for touch IDs that returned true in touchBegan
    // so we need to implement both methods (there's also touchMoved which works the same way)
    bool touchBegan( ci::app::TouchEvent::Touch touch );
	bool touchMoved( ci::app::TouchEvent::Touch touch );
    bool touchEnded( ci::app::TouchEvent::Touch touch );

	void loadXml( ci::XmlTree &xml );

	void setProperty( const XNodeStateProperty& prop );
    
    // we'll implement hitTest as a helper for touchBegan/touchEnded
    bool hitTest( const ci::Vec2f &screenPos );

	// setters
	virtual void setSize( const ci::Vec2f &size )				{ mWidth = size.x; mHeight = size.y; }
	virtual ci::ColorA getColor() const 						{ return mColor; }
	virtual void setColor( const ci::ColorA &color )			{ mColor = color; }

	virtual float getWidth() const								{ return mWidth; }
	virtual void setWidth(float value)							{ mWidth = value; }
	virtual float getHeight() const								{ return mHeight; }
	virtual void setHeight(float value)							{ mHeight = value; }
    
protected:
	XRect() : mColor(ci::Color::white()) {}
    
	ci::Anim<float> mWidth;
	ci::Anim<float> mHeight;
	ci::ColorA mColor;
};

}