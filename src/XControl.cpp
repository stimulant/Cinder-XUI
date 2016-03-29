#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIO.h"
#include "XControl.h"

using namespace ci;
using namespace xui;

XControlRef XControl::create()
{
    XControlRef ref = XControlRef( new XControl() );
    return ref;
}

XControlRef XControl::create( ci::XmlTree &xml )
{
	XControlRef ref = XControl::create();
	ref->loadXml( xml );
	return ref;
}

void XControl::draw(float opacity)
{
    // Matrix is already applied so we can draw at origin
    gl::color( mColor * ColorA(1.0f, 1.0f, 1.0f, mOpacity * opacity) );
    XRect::draw(opacity);
    if (mScene)
        mScene->deepDraw();
}

bool XControl::mouseDown( ci::app::MouseEvent event )
{
    event.setPos(event.getPos() + ivec2(mX, mY));
    
    if (mScene)
        return mScene->mouseDown(event);
    return false; // only true if all touches were consumed
}

bool XControl::mouseUp( ci::app::MouseEvent event )
{
    event.setPos(event.getPos() + ivec2(mX, mY));
    
    if (mScene)
        return mScene->mouseUp(event);
    return false; // only true if all touches were consumed
}

bool XControl::mouseDrag( ci::app::MouseEvent event )
{
    event.setPos(event.getPos() + ivec2(mX, mY));
    
    if (mScene)
        return mScene->mouseDrag(event);
    return false; // only true if all touches were consumed
}

bool XControl::touchesBegan( ci::app::TouchEvent event )
{
    ci::app::TouchEvent newEvent;
    for (auto& touch : event.getTouches())
    {
        newEvent.getTouches().push_back(
                                        ci::app::TouchEvent::Touch(touch.getPos() + vec2(mX, mY),
                                                          touch.getPrevPos() + vec2(mX, mY),
                                                          touch.getId(), touch.getTime(), NULL));
    }
    
    if (mScene)
        return mScene->touchesBegan(newEvent);
    return false; // only true if all touches were consumed
}

bool XControl::touchesMoved( ci::app::TouchEvent event )
{
    ci::app::TouchEvent newEvent;
    for (auto& touch : event.getTouches())
    {
        newEvent.getTouches().push_back(
                                        ci::app::TouchEvent::Touch(touch.getPos() + vec2(mX, mY),
                                                          touch.getPrevPos() + vec2(mX, mY),
                                                          touch.getId(), touch.getTime(), NULL));
    }
    
    if (mScene)
        return mScene->touchesMoved(newEvent);
    return false; // only true if all touches were consumed
}

bool XControl::touchesEnded( ci::app::TouchEvent event )
{
    ci::app::TouchEvent newEvent;
    for (auto& touch : event.getTouches())
    {
        newEvent.getTouches().push_back(
                                        ci::app::TouchEvent::Touch(touch.getPos() + vec2(mX, mY),
                                                          touch.getPrevPos() + vec2(mX, mY),
                                                          touch.getId(), touch.getTime(), NULL));
    }
    
    if (mScene)
        return mScene->touchesEnded(newEvent);
    return false; // only true if all touches were consumed
}

void XControl::loadXml( ci::XmlTree &xml )
{
	XRect::loadXml(xml);

	// get/set properties from xml
	if (xml.hasAttribute("scene"))
	{
        DataSourceRef dataSource = app::loadAsset( xml.getAttributeValue<std::string>("scene") );
        XSceneRef sceneRef;
        std::string xuiString;
        xuiString.assign((char*)(dataSource->getBuffer()->getData()), dataSource->getBuffer()->getSize());
        
        // try to load this asset
        mScene = XScene::createFromXmlString(xuiString);
	}
}