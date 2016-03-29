#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIO.h"
#include "XButton.h"
#include "XScene.h"

using namespace ci;
using namespace xui;

XButtonRef XButton::create()
{
    XButtonRef ref = XButtonRef( new XButton() );
    return ref;
}

XButtonRef XButton::create( ci::XmlTree &xml )
{
	XButtonRef ref = XButton::create();
	ref->loadXml( xml );
	return ref;
}

void XButton::draw(float opacity)
{
    // Matrix is already applied so we can draw at origin
    gl::color( mColor * ColorA(1.0f, 1.0f, 1.0f, mOpacity * opacity) );
	switch (mState)
	{
		case State::DEFAULT:
			gl::draw(mDefaultTexture);
			break;
		case State::PRESS:
			gl::draw(mPressTexture);
			break;
	}
    
    // and then any children will be draw after this
}

void XButton::loadXml( ci::XmlTree &xml )
{
	XRect::loadXml(xml);

	// get/set properties from xml
	if (xml.hasAttribute("default"))
	{
		mDefaultTexture = gl::Texture::create(loadImage(app::loadAsset(xml.getAttributeValue<std::string>("default"))));

		// if width and height have not been set, default to default texture's size
		if (mWidth == 0.0f && mHeight == 0.0f)
		{
			mWidth = (float)mDefaultTexture->getWidth();
			mHeight = (float)mDefaultTexture->getHeight();
		}
	}
	if ( xml.hasAttribute( "press" ) )
		mPressTexture = gl::Texture::create( loadImage( app::loadAsset( xml.getAttributeValue<std::string>( "press" ) ) ) );
    if ( xml.hasAttribute( "setState" ) )
        mSetState = xml.getAttributeValue<std::string>( "setState" );
    if ( xml.hasAttribute( "playSound" ) )
    {
        mPlaySound = xml.getAttributeValue<std::string>( "playSound" );
        getAudioVoice( mPlaySound );
    }
}

bool XButton::mouseDownInternal(ci::app::MouseEvent event)
{
	bool hit = XRect::mouseDownInternal(event);
	if (hit)
	{
		if (mScript)
			mScript->call("press");
		mState = State::PRESS;
        
        // globally set the state of the scene if we have one
        if (mSetState != "")
        {
            std::shared_ptr<XScene> ptr = mRoot.lock();
            if (ptr)
                ptr->setState( mSetState );
        }
        if (mPlaySound != "")
        {
            playSound( mPlaySound );
        }
	}
	return hit;
}

bool XButton::mouseDragInternal(ci::app::MouseEvent event)
{
	bool value = XRect::mouseDragInternal(event);
	return value;
}

bool XButton::mouseUpInternal(ci::app::MouseEvent event)
{
	bool value = XRect::mouseUpInternal(event);
	mState = State::DEFAULT;
    if (mScript)
        mScript->call("release");
	return value;
}

bool XButton::touchBeganInternal(ci::app::TouchEvent::Touch touch)
{
	bool hit = XRect::touchBeganInternal(touch);
	if (hit)
	{
		if (mScript)
			mScript->call("press");
		mState = State::PRESS;
	}
	return hit;
}

bool XButton::touchMovedInternal(ci::app::TouchEvent::Touch touch)
{
	bool value = XRect::touchMovedInternal(touch);
	return value;
}

bool XButton::touchEndedInternal(ci::app::TouchEvent::Touch touch)
{
	bool value = XRect::touchEndedInternal(touch);	
	mState = State::DEFAULT;
    if (mScript)
        mScript->call("release");
	return value;
}