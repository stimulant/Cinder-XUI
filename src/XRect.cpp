#include "XRect.h"
#include "XScript.h"
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"
#include "cinder/Quaternion.h"
#include "cinder/Path2d.h"

using namespace ci;
using namespace ci::app;
using namespace xui;

XRect::XRect()
{
	mWidth = 0.0f;
	mHeight = 0.0f;
	mColor = ci::Color::white();
    mStrokeColor = ColorA(0.0f, 0.0f, 0.0f, 0.0f);
    mCornerRadius = 0.0f;

	// setup guestures
	mGuestureTouchDelay = 0.07;
	mGuestureTouchTime = 0.0;
	mInterpolationSpeed = 0.33f;
	
	mPan = vec2(0.0f);
	mPanMax = vec2(std::numeric_limits<float>::max());
	mPanMin = vec2(-std::numeric_limits<float>::max());
	mPanSpeed = vec2(1.0f);
	mPanTarget = mPan;
	mPanThreshold = vec2(1.0f);
	mRotationSpeed = -2.5f;
	mRotationThreshold = 0.005f;

	mScale = vec2(1.0f);
	mScaleMax = vec2(std::numeric_limits<float>::max());
	mScaleMin = vec2(0.0f);
	mScaleSpeed = vec2(0.0067f);
	mScaleSymmetry = true;
	mScaleTarget = mScale;
	mScaleThreshold = vec2(1.0f);

	mTapDelay = 1.0;
	mTapPosition = vec2(std::numeric_limits<float>::min());
	mTapCount = 0;
	mTapTime = 0.0;
	mTapThreshold = 200.0f;
	mTouchDelay = 0.07;
	mTouchTime = 0.0;

	mPanEnabled = false;
	mRotationEnabled = false;
	mScaleEnabled = false;
}

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
    // draw fill
    gl::color( mColor * ColorA(1.0f, 1.0f, 1.0f, mOpacity * opacity) );
    if (mCornerRadius > 0.0f)
        gl::drawSolidRoundedRect(Rectf(vec2(0,0), vec2(mWidth, mHeight)), mCornerRadius);
    else
        gl::drawSolidRect(Rectf(vec2(0,0), vec2(mWidth, mHeight)));
    
    // draw stroke
    if (mStrokeColor != ColorA(0.0f, 0.0f, 0.0f, 0.0f))
    {
        gl::color( mStrokeColor * ColorA(1.0f, 1.0f, 1.0f, mOpacity * opacity) );
        if (mCornerRadius > 0.0f)
            gl::drawStrokedRoundedRect(Rectf(vec2(0,0), vec2(mWidth, mHeight)), mCornerRadius);
        else
            gl::drawStrokedRect(Rectf(vec2(0,0), vec2(mWidth, mHeight)));
    }
    
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
	XNode::loadXml(xml);

	// get/set properties from xml
	mWidth = xml.getAttributeValue( "width", 0.0f );
	mHeight = xml.getAttributeValue( "height", 0.0f );
	mPanEnabled = (xml.getAttributeValue("panEnabled", 0) == 1 || xml.getAttributeValue<std::string>("panEnabled", "") == "true");
	mScaleEnabled = (xml.getAttributeValue("scaleEnabled", 0) == 1 || xml.getAttributeValue<std::string>("scaleEnabled", "") == "true");
	mRotationEnabled = (xml.getAttributeValue("rotateEnabled", 0) == 1 || xml.getAttributeValue<std::string>("rotateEnabled", "") == "true");
	std::string color = xml.getAttributeValue< std::string >( "color", "0xffffffff" );
	setColor(hexToColor(color));
    std::string strokeColor = xml.getAttributeValue< std::string >( "strokeColor", "0x00000000" );
    setStrokeColor(hexToColor(strokeColor));
    mCornerRadius = xml.getAttributeValue( "cornerRadius", 0.0f );
}

void XRect::setProperty( const XNodeStateProperty& prop )
{
	if (prop.mType == "color")
		setColor( hexToColor(prop.mValue.c_str()) );
    if (prop.mType == "strokeColor")
        setStrokeColor( hexToColor(prop.mValue.c_str()) );
	if (prop.mType == "width")
		app::timeline().apply( &mWidth, (float)atof(prop.mValue.c_str()), prop.mTime, prop.mEaseFn );
	if (prop.mType == "height")
		app::timeline().apply( &mHeight, (float)atof(prop.mValue.c_str()), prop.mTime, prop.mEaseFn );
	else
		XNode::setProperty( prop );
}

bool XRect::hitTest( const ci::vec2 &screenPos )
{
    vec2 pos = globalToLocal(screenPos);
	return Area(vec2(0, 0), vec2(mWidth, mHeight)).contains( pos );
}

void XRect::addGuestureTouch(const TouchEvent::Touch& touch)
{
	mGuestureTouchTime = getElapsedSeconds();
	bool found = false;
	for (const TouchEvent::Touch& iter : mGuestureTouches) {
		if (iter.getId() == touch.getId()) {
			found = true;
		}
	}
	if (!found) {
		mGuestureTouches.push_back(touch);
	}
}

void XRect::guesturesBeganInternal(ci::app::TouchEvent event) 
{
	// recurse
	for (XNodeRef &node : mChildren)
		node->guesturesBeganInternal(event);

	if (mTapEnabled) 
	{
		for (const TouchEvent::Touch& touch : event.getTouches()) 
		{
			if (hitTest(touch.getPos()))
			{
				mTapPosition = touch.getPos();
				mTapTime = getElapsedSeconds();
				mGuestureTouches = { touch };
				break;
			}
		}
	}
}

float wrapAngle(float v)
{
	static const float pi = (float)M_PI;
	static const float twoPi = pi * 2.0f;

	v = fmod(v + pi, twoPi);
	if (v < 0.0f) {
		v += twoPi;
	}
	v -= pi;
	return v;
}

void XRect::guesturesMovedInternal(TouchEvent event) 
{
	// recurse
	for (XNodeRef &node : mChildren)
		node->guesturesMovedInternal(event);

	mGuestureTouches.clear();

	// handle tap
	if (mTapEnabled && mTapPosition != vec2(std::numeric_limits<float>::min()))
	{
		bool tapped = false;
		for (const TouchEvent::Touch& touch : event.getTouches()) 
		{
			bool hit = hitTest(touch.getPos());
			float tapDistance = glm::distance(touch.getPos(), mTapPosition);
			if (hit && tapDistance < mTapThreshold) 
			{
				tapped = true;
				addGuestureTouch(touch);
				break;
			}
			else
			{
				app::console() << "not tapped" << std::endl;
			}
		}
		if (!tapped) 
		{
			// reset tap
			mTapPosition = vec2(std::numeric_limits<float>::min());
			mTapTime = 0.0;
			mTapCount = 0;
		}
	}

	// handle pan/rotate/scale
	const vec2 panSpeed(mPanSpeed.x * pow((mScaleMax.x + mScaleMin.x) - mScale.x, 0.0002f), mPanSpeed.y * pow((mScaleMax.y + mScaleMin.y) - mScale.y, 0.0002f));

	bool applyPan = false;
	bool applyRotation = false;
	bool applyScale = false;
	float panX = 0.0f;
	float panY = 0.0f;
	float scaleX = 0.0f;
	float scaleY = 0.0f;
	float rotation = 0.0f;

	// pan
	if (mPanEnabled) 
	{
		for (const TouchEvent::Touch& touch : event.getTouches()) 
		{
			const vec2 a(touch.getPos());
			const vec2 b(touch.getPrevPos());
			if (hitTest(a))
			{
				panX = a.x - b.x;
				panY = a.y - b.y;
				addGuestureTouch(touch);
				break;
			}
		}
	}

	// create array of touches inside
	std::vector<TouchEvent::Touch> insideTouches;
	for (auto touch : event.getTouches())
	{
		if (hitTest(touch.getPrevPos()))
			insideTouches.push_back(touch);
	}

	// scale/rotate
	if (insideTouches.size() > 1)
	{
		const vec2 ap0(insideTouches[0].getPos());
		const vec2 ap1(insideTouches[0].getPrevPos());
		const vec2 bp0(insideTouches[1].getPos());
		const vec2 bp1(insideTouches[1].getPrevPos());
		if (hitTest(bp0) && hitTest(bp1)) 
		{
			// Scale
			if (mScaleEnabled)
			{
				scaleX = glm::distance(ap0.x, bp0.x) - glm::distance(ap1.x, bp1.x);
				scaleY = glm::distance(ap0.y, bp0.y) - glm::distance(ap1.y, bp1.y);
			}

			// Rotation
			if (mRotationEnabled)
				rotation = wrapAngle(atan2(ap0.y - bp0.y, ap0.x - bp0.x) - atan2(ap1.y - bp1.y, ap1.x - bp1.x));
		}
	}

	std::vector<Motion> motions = {
		{ MotionType_PanX, fabsf(panX) / mPanThreshold.x },
		{ MotionType_PanY, fabsf(panY) / mPanThreshold.y },
		{ MotionType_Rotation, fabsf(rotation) / mRotationThreshold },
		{ MotionType_ScaleX, fabsf(scaleX) / mScaleThreshold.x },
		{ MotionType_ScaleY, fabsf(scaleY) / mScaleThreshold.y },
	};

	for (const Motion& motion : motions)
	{
		MotionType t = motion.first;
		if (motion.second > 1.0f) 
		{
			if (t == MotionType_PanX || t == MotionType_PanY) 
				applyPan = true;
			else if (t == MotionType_Rotation)
				applyRotation = true;
			else if (t == MotionType_ScaleX || t == MotionType_ScaleY)
				applyScale = true;
		}
	}
		
	if (insideTouches.size() > 1 && (applyPan || applyRotation || applyScale)) {
		addGuestureTouch(insideTouches[0]);
		addGuestureTouch(insideTouches[1]);
	}

	if (applyPan) 
	{
		mPanTarget.x += panX * panSpeed.x;
		mPanTarget.y += panY * panSpeed.y;
	}
	if (applyRotation)
		mRotationTarget.z += rotation * mRotationSpeed;
	if (applyScale) 
	{
		if (mScaleSymmetry) 
			mScaleTarget += vec2((scaleX * mScaleSpeed.x + scaleY * mScaleSpeed.y) * 0.5f);
		else
			mScaleTarget += vec2(scaleX * mScaleSpeed.x, scaleY * mScaleSpeed.y);
	}
}

void XRect::guesturesEndedInternal(TouchEvent event) 
{
	// recurse
	for (XNodeRef &node : mChildren)
		node->guesturesEndedInternal(event);

	if (mTapEnabled && mTapTime > 0.0) 
	{
		for (const TouchEvent::Touch& touch : event.getTouches()) 
		{
			if (hitTest(touch.getPos()) && glm::distance(touch.getPos(), mTapPosition) < mTapThreshold)
			{
				mTapCount++;

				// lua update
				if (mScript)
					mScript->call("tap", mTapCount);

				mTapPosition = touch.getPos();
				mTapTime = app::getElapsedSeconds();
				mGuestureTouches = { touch };
				break;
			}
		}
	}
}

void XRect::guesturesUpdate()
{
	if (mPanEnabled) 
	{
		mPanTarget = glm::clamp(mPanTarget, mPanMin, mPanMax);
		mPan = glm::mix(mPan, mPanTarget, mInterpolationSpeed);
	}
	if (mRotationEnabled) 
		mRotation = glm::slerp(mRotation, mRotationTarget, mInterpolationSpeed);
	if (mScaleEnabled) 
	{
		mScaleTarget = glm::clamp(mScaleTarget, mScaleMin, mScaleMax);
		mScale = glm::mix(mScale, mScaleTarget, mInterpolationSpeed);
	}
	mGuesturesMatrix = mat4(1.0f);
	mGuesturesMatrix = glm::translate(mGuesturesMatrix, vec3(mPan, 0.0f));
	mGuesturesMatrix = glm::translate(mGuesturesMatrix, vec3(mWidth/2.0f, mHeight/2.0f, 0.0f));
	mGuesturesMatrix = glm::rotate(mGuesturesMatrix, -mRotation.z, vec3(0.0f, 0.0f, 1.0f));
	mGuesturesMatrix = glm::scale(mGuesturesMatrix, vec3(mScale, 1.0f));
	mGuesturesMatrix = glm::translate(mGuesturesMatrix, -vec3(mWidth/2.0f, mHeight/2.0f, 0.0f));

	double elapsedTapTime = getElapsedSeconds() - mTapTime;
	if (!mTapEnabled || (mTapTime > 0.0 && elapsedTapTime > mTapDelay))
	{
		// reset tap
		mTapPosition = vec2(std::numeric_limits<float>::min());
		mTapTime = 0.0;
		mTapCount = 0;
	}
}