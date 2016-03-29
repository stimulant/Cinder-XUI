#pragma once
#include "XRect.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/gl/Texture.h"

namespace xui {

typedef std::shared_ptr<class XButton> XButtonRef;

class XButton : public XRect
{
public:
	enum class State
	{
		DISABLED,
		DEFAULT,
		OVER,
		PRESS
	};

    static XButtonRef create();
    static XButtonRef create( ci::XmlTree &xml );

	virtual XNode::NodeType getType() { return XNode::NodeTypeButton; }
    
    void draw(float opacity = 1.0f);

	bool mouseDownInternal(ci::app::MouseEvent event);
	bool mouseDragInternal(ci::app::MouseEvent event);
	bool mouseUpInternal(ci::app::MouseEvent event);

	// touchEnded will only be called for touch IDs that returned true in touchBegan
	// so we need to implement both methods (there's also touchMoved which works the same way)
	bool touchBeganInternal(ci::app::TouchEvent::Touch touch);
	bool touchMovedInternal(ci::app::TouchEvent::Touch touch);
	bool touchEndedInternal(ci::app::TouchEvent::Touch touch);

	void loadXml( ci::XmlTree &xml );
	void setDefaultTexture( ci::gl::TextureRef texture ) { mDefaultTexture = texture; }
	void setPressTexture( ci::gl::TextureRef texture ) { mPressTexture = texture; }
    
protected:
	XButton() { mState = State::DEFAULT; }
    
	State mState;
    ci::gl::TextureRef mDefaultTexture;
	ci::gl::TextureRef mPressTexture;
    std::string        mSetState;
    std::string        mPlaySound;
};

}