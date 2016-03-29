#pragma once
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/gl/Texture.h"

#include "XRect.h"
#include "XImage.h"

namespace xui {


typedef std::shared_ptr<class XCarousel> XCarouselRef;

class XCarousel : public XRect
{
public:
    static XCarouselRef create();
    static XCarouselRef create( ci::XmlTree &xml );

    virtual XNode::NodeType getType() { return XNode::NodeTypeCarousel; }
    
	void update(double elapsedSeconds);
    void draw(float opacity = 1.0f);

	// touchEnded will only be called for touch IDs that returned true in touchBegan
	// so we need to implement both methods (there's also touchMoved which works the same way)
	bool touchBeganInternal(ci::app::TouchEvent::Touch touch);
	bool touchMovedInternal(ci::app::TouchEvent::Touch touch);
	bool touchEndedInternal(ci::app::TouchEvent::Touch touch);

	void loadXml( ci::XmlTree &xml );

	virtual bool getCarouselEnabled() const		{ return mCarouselEnabled; }
	virtual void setCarouselEnabled(bool carouselEnabled) { mCarouselEnabled = carouselEnabled; }
    
protected:
	XCarousel();

	void layoutSlides();
	size_t previous(size_t current) const;
	size_t next(size_t current) const;
	void previousSlide();
	void nextSlide();
    
	bool								mCarouselEnabled;
	std::vector<XImageRef>				mSlides;
	float								mSlideWidth;

	size_t								mCurrentIndex;
	ci::TimelineRef						mNextTimeline;		//! timeline for animating to next panel
	ci::TimelineRef						mPreviousTimeline;	//! timeline for animating to previous panel

	// touches
	ci::vec2							mFirstTouchPosition;
	float								mFirstTouchTime;

	// all images that are currently in view
	std::vector<XImageRef>				mActiveImages;
	float								mImageWidth;
	ci::Anim<float>						mTime;
	float								mPreviousTime;

	// properties
	float								mTransitionDuration;
	float								mTransitionThreshold;
	float								mTouchMotionScale;
	float								mSlideMargin;
	bool								mCanMoveToNext;
	bool								mCanMoveToPrevious;
	float								mTouchEasePower;
	float								mSideImageAlpha;
};

}