#include "XRect.h"
#include "XScript.h"
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"

using namespace ci;
using namespace xui;

XRectRef XRect::create()
{
    XRectRef ref = XRectRef( new XRect() );
    return ref;
}

XRectRef XRect::create(ci::XmlTree &xml)
{
	XRectRef ref = XRect::create();
	ref->loadXml(xml);
	return ref;
}

void XRect::draw(float opacity)
{
    // Matrix is already applied so we can draw at origin
    gl::color( mColor * ColorA(1.0f, 1.0f, 1.0f, mOpacity * opacity) );
	gl::drawSolidRect(Rectf(Vec2f(0,0), Vec2f(mWidth, mHeight)));
    
    // and then any children will be draw after this
}

bool XRect::mouseDownInternal( ci::app::MouseEvent event )
{
	bool hit = hitTest( event.getPos() );
	if (hit && mScript)
		mScript->call("mouseDown", event.getPos().x, event.getPos().y);
    return hit;
}

bool XRect::mouseDragInternal( ci::app::MouseEvent event )
{
	if (mScript)
		mScript->call("mouseDrag", event.getPos().x, event.getPos().y);
	return true;
}

bool XRect::mouseUpInternal( ci::app::MouseEvent event )
{
	if (mScript)
		mScript->call("mouseUp", event.getPos().x, event.getPos().y);
    return true;
}

bool XRect::touchBeganInternal( ci::app::TouchEvent::Touch touch )
{
	bool hit = hitTest( touch.getPos() );
	if (hit && mScript)
		mScript->call("touchBegan", touch.getPos().x, touch.getPos().y);
    return hit;
}

bool XRect::touchMovedInternal( ci::app::TouchEvent::Touch touch )
{
	if (mScript)
		mScript->call("touchMoved", touch.getPos().x, touch.getPos().y);
	return true;
}

bool XRect::touchEndedInternal( ci::app::TouchEvent::Touch touch )
{
	if (mScript)
		mScript->call("touchEnded", touch.getPos().x, touch.getPos().y);
    return true;
}

void XRect::loadXml(ci::XmlTree &xml)
{
	// get/set properties from xml
	mWidth = xml.getAttributeValue( "width", 0.0f );
	mHeight = xml.getAttributeValue( "height", 0.0f );
	std::string color = xml.getAttributeValue< std::string >( "color", "0xffffffff" );
	setColor(hexToColor(color));

	XNode::loadXml(xml);
}

void XRect::setProperty( const XNodeStateProperty& prop )
{
	if (prop.mType == "color")
		setColor( hexToColor(prop.mValue.c_str()) );
	if (prop.mType == "width")
		app::timeline().apply( &mWidth, (float)atof(prop.mValue.c_str()), prop.mTime, prop.mEaseFn );
	if (prop.mType == "height")
		app::timeline().apply( &mHeight, (float)atof(prop.mValue.c_str()), prop.mTime, prop.mEaseFn );
	else
		XNode::setProperty( prop );
}

bool XRect::hitTest( const ci::Vec2f &screenPos )
{
    Vec2f pos = globalToLocal(screenPos);
	return Area(Vec2f(0, 0), Vec2f(mWidth, mHeight)).contains( pos );
}