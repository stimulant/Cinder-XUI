#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIO.h"
#include "XCarousel.h"

#include <boost/algorithm/string.hpp>

using namespace ci;
using namespace xui;

XCarousel::XCarousel() 
	: mCurrentIndex(0), mTime(0.0), mNextTimeline(Timeline::create()), mPreviousTimeline(Timeline::create()), mCarouselEnabled(true)
{
	mNextTimeline->setInfinite(false);
	mNextTimeline->setDefaultAutoRemove(false);
	mPreviousTimeline->setInfinite(false);
	mPreviousTimeline->setDefaultAutoRemove(false);
}

XCarouselRef XCarousel::create()
{
    XCarouselRef ref = XCarouselRef( new XCarousel() );
    return ref;
}

XCarouselRef XCarousel::create( ci::XmlTree &xml )
{
	XCarouselRef ref = XCarousel::create();
	ref->loadXml( xml );
	return ref;
}

void XCarousel::update(double elapsedSeconds)
{
	// update timelines
	if (mTime >= 0.0) 
	{
		if (mPreviousTimeline->getCurrentTime() != 0.0f)
			mPreviousTimeline->stepTo(0.0f);
		mNextTimeline->stepTo(mTime);
	}
	else 
	{
		if (mNextTimeline->getCurrentTime() != 0.0f)
			mNextTimeline->stepTo(0.0f);
		mPreviousTimeline->stepTo(-mTime);
	}

	// update previous time
	if (mPreviousTime != mTime) 
	{
		mPreviousTime = mTime;
		//mSignalSlideDidMove();
	}
}

void XCarousel::draw(float opacity)
{
    // Matrix is already applied so we can draw at origin
    gl::color( mColor * ColorA(1.0f, 1.0f, 1.0f, mOpacity * opacity) );
    
    // and then any children will be draw after this
	if (mCarouselEnabled)
	{
		for (const auto &img : mActiveImages)
		{
			img->deepDraw(opacity);
		}
	}
	else
	{
		// otherwise only draw center
		if (!mActiveImages.empty())
		{
			const auto img = *(mActiveImages.end() - 1);
			img->deepDraw(opacity);
		}
	}
}

void XCarousel::loadXml( ci::XmlTree &xml )
{
	// move all XImage children out of normal scene graph before we do standard loading
	XmlTree::Container::iterator i = xml.getChildren().begin();
	while (i != xml.getChildren().end())
	{
		// only allow image children for now
		if (boost::iequals((*i)->getTag(), "Image"))
		{
			XImageRef image = XImage::create(*(*i));
			mSlides.push_back(image);
			mSlideWidth = image->getWidth();

			// remove this from the xml tree so it doesn't get loaded as a normal child
			i = xml.getChildren().erase(i);
		}
		else
			++i;
	}

	// get/set carousel properties from xml
	mTransitionDuration = xml.getAttributeValue("transitionDuration", 0.66f);
	mTouchMotionScale = xml.getAttributeValue("touchMotionScale", 1.5f);
	mTransitionThreshold = xml.getAttributeValue("transitionThreshold", 0.33f);
	mSlideMargin = xml.getAttributeValue("slideMargin", 0.0f);
	mCanMoveToPrevious = (xml.getAttributeValue("canMoveToPrevious", 1) == 1);
	mCanMoveToNext = xml.getAttributeValue("canMoveToNext", 1) == 1;
	mTouchEasePower = xml.getAttributeValue("touchEasePower", 1.0f);
	mSideImageAlpha = xml.getAttributeValue("sideImageAlpha", 1.0f);
    std::string carouselEnabledStr = xml.getAttributeValue<std::string>("carouselEnabled", "true");
    mCarouselEnabled = (carouselEnabledStr == "true" || carouselEnabledStr == "1");

	// load this rect normally (do this last so we can remove all the image children)
	XRect::loadXml(xml);

	// layout slides
	layoutSlides();
}

void XCarousel::layoutSlides()
{
	mNextTimeline->clear();
	mPreviousTimeline->clear();
	mNextTimeline->stepTo(0.0);
	mPreviousTimeline->stepTo(0.0);
	mActiveImages.clear();

	if (mSlides.empty())
		return;

	// current set of content
	const auto &center = mSlides[mCurrentIndex];
	const auto &left = mSlides[previous(mCurrentIndex)];
	const auto &farLeft = mSlides[previous(previous(mCurrentIndex))];
	const auto &right = mSlides[next(mCurrentIndex)];
	const auto &farRight = mSlides[next(next(mCurrentIndex))];

	// positions of things
	mImageWidth = center->getWidth();
	const float margin = mSlideMargin;
	const float w = getWidth();
	const float cx = w / 2.0f;
	const float cy = getHeight() / 2.0f;

	const ci::vec2 centerPosition(cx, cy);
	const ci::vec2 leftPosition(cx - (mImageWidth + margin), cy);
	const ci::vec2 rightPosition(cx + (mImageWidth + margin), cy);
	const ci::vec2 leftVanishPosition(leftPosition.x - (mImageWidth + margin), cy);
	const ci::vec2 rightVanishPosition(rightPosition.x + (mImageWidth + margin), cy);

	//auto center = make_shared<TransformedImage>(page, -page->getSize() * 0.5f);
	//auto left = make_shared<TransformedImage>(pageLeft, -pageLeft->getSize() * 0.5f);
	//auto right = make_shared<TransformedImage>(pageRight, -pageRight->getSize() * 0.5f);
	//auto farRight = make_shared<TransformedImage>(pageFarRight, -pageFarRight->getSize() * 0.5f);
	//auto farLeft = make_shared<TransformedImage>(pageFarLeft, -pageFarLeft->getSize() * 0.5f);

	center->setX(centerPosition.x);
	right->setX(rightPosition.x);
	right->setOpacity(mSideImageAlpha);
	left->setX(leftPosition.x);
	left->setOpacity(mSideImageAlpha);
	farLeft->setX(leftVanishPosition.x);
	farLeft->setOpacity(0.0f);
	farRight->setX(rightVanishPosition.x);
	farRight->setOpacity(0.0f);

	// all times here are normalized; the total second duration is set in json
	// TODO: make actual, quality animations
	const auto ease = EaseInOutQuad();

	// We need to tween on raw pointers so the Anim->Timeline parenting behavior doesn't blow away
	// these tweens when we add the properties to the other timeline
	mNextTimeline->appendToPtr(right->getXAnim()->ptr(), centerPosition.x, 1.0f, ease);
	mNextTimeline->appendToPtr(right->getOpacityAnim()->ptr(), 1.0f, 1.0f, ease);
	mNextTimeline->appendToPtr(center->getXAnim()->ptr(), leftPosition.x, 1.0f, ease);
	mNextTimeline->appendToPtr(center->getOpacityAnim()->ptr(), mSideImageAlpha, 1.0f, ease);
	mNextTimeline->appendToPtr(left->getXAnim()->ptr(), leftVanishPosition.x, 1.0f, ease);
	mNextTimeline->appendToPtr(left->getOpacityAnim()->ptr(), 0.0f, 0.5f, ease).delay(0.25f); // fade out
	mNextTimeline->appendToPtr(farRight->getXAnim()->ptr(), rightPosition.x, 1.0f, ease);
	mNextTimeline->appendToPtr(farRight->getOpacityAnim()->ptr(), mSideImageAlpha, 0.5f, ease).delay(0.2f); // fade in
	
	mPreviousTimeline->appendToPtr(left->getXAnim()->ptr(), centerPosition.x, 1.0f, ease);
	mPreviousTimeline->appendToPtr(left->getOpacityAnim()->ptr(), 1.0f, 1.0f, ease);
	mPreviousTimeline->appendToPtr(center->getXAnim()->ptr(), rightPosition.x, 1.0f, ease);
	mPreviousTimeline->appendToPtr(center->getOpacityAnim()->ptr(), mSideImageAlpha, 1.0f, ease);
	mPreviousTimeline->appendToPtr(right->getXAnim()->ptr(), rightVanishPosition.x, 1.0f, ease);
	mPreviousTimeline->appendToPtr(right->getOpacityAnim()->ptr(), 0.0f, 0.5f, ease).delay(0.25f); // fade out
	mPreviousTimeline->appendToPtr(farLeft->getXAnim()->ptr(), leftPosition.x, 1.0f, ease);
	mPreviousTimeline->appendToPtr(farLeft->getOpacityAnim()->ptr(), mSideImageAlpha, 0.5f, ease).delay(0.2f); // fade in
	
	// store references to images
	mActiveImages.push_back(farLeft);
	mActiveImages.push_back(farRight);
	mActiveImages.push_back(left);
	mActiveImages.push_back(right);
	mActiveImages.push_back(center);
}

size_t XCarousel::previous(size_t current) const
{
	if (current == 0)
		current = mSlides.size() - 1;
	else
		current -= 1;
	return current;
}

size_t XCarousel::next(size_t current) const
{
	if (current >= mSlides.size() - 1)
		current = 0;
	else
		current += 1;
	return current;
}

void XCarousel::previousSlide()
{
	const float duration = math<float>::abs(-1.0f - mTime) * mTransitionDuration;
	
	app::timeline().apply(&mTime, -1.0f, duration)
		.finishFn([this]() 
		{
			mCurrentIndex = previous(mCurrentIndex);
			layoutSlides();
			mTime = 0.0;

			// tell lua we have changed our index
			if (mScript)
				mScript->call("slide", mCurrentIndex);
		}
	);
}

void XCarousel::nextSlide()
{
	const float duration = (1.0f - mTime) * mTransitionDuration;
	app::timeline().apply(&mTime, 1.0f, duration)
		.finishFn([this]() 
		{
			mCurrentIndex = next(mCurrentIndex);
			layoutSlides();
			mTime = 0.0;

			if (mScript)
				mScript->call("slide", mCurrentIndex);
		}
	);
}

bool XCarousel::touchBeganInternal(ci::app::TouchEvent::Touch touch)
{
	if (!mCarouselEnabled)
		return false;

	app::console() << touch.getPos().x << ", " << touch.getPos().y << std::endl;
	bool hit = XRect::touchBeganInternal(touch);

	if (hit)
	{
		if (mSlides.empty())
			return hit;

		auto timeline = mTime.getParent();
		if (timeline) 
		{
			mTime.stop(); // Stop the current anim, so that it doesn't complete
			timeline->removeTarget(&mTime);

		}
		mFirstTouchPosition = touch.getPos();
		mFirstTouchTime = mTime;
		//mSignalSomeoneTouchedMe();
	}

	return hit;
}

bool XCarousel::touchMovedInternal(ci::app::TouchEvent::Touch touch)
{
	if (!mCarouselEnabled)
		return false;

	bool value = XRect::touchMovedInternal(touch);

	if (mSlides.empty())
		return value;

	auto p = touch.getPos();
	float dt = -(p.x - mFirstTouchPosition.x) / (mTouchMotionScale * mImageWidth);
	const float mag = math<float>::abs(dt);
	const float sign = dt < 0.0f ? -1.0f : 1.0f;
	dt = math<float>::pow(mag, mTouchEasePower) * sign;
	const float leftLimit = mCanMoveToPrevious ? -1.0f : 0.0f;
	const float rightLimit = mCanMoveToNext ? 1.0f : 0.0f;
	mTime = math<float>::clamp(mFirstTouchTime + dt, leftLimit, rightLimit);

	//app::console() << "moved: " << mTime << std::endl;

	return value;
}

bool XCarousel::touchEndedInternal(ci::app::TouchEvent::Touch touch)
{
	if (!mCarouselEnabled)
		return false;

	bool value = XRect::touchEndedInternal(touch);

	if (mSlides.empty())
		return value;

	if (mTime > mTransitionThreshold)
		nextSlide();
	else if (mTime < -mTransitionThreshold)
		previousSlide();
	else 
	{
		const float duration = math<float>::abs(mTime) * mTransitionDuration;
		app::timeline().apply(&mTime, 0.0f, duration);
	}

	return value;
}