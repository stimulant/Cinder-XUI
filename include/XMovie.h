#pragma once
#include "XRect.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/gl/Texture.h"
#include "cinder/qtime/QuickTimeGl.h"

namespace xui {

typedef std::shared_ptr<class XMovie> XMovieRef;

class XMovie : public XRect
{
public:
    static XMovieRef create();
    static XMovieRef create( ci::XmlTree &xml );

	virtual XNode::NodeType getType() { return XNode::NodeTypeMovie; }
    
    void update( double elapsedSeconds );
    void draw(float opacity = 1.0f);

	void loadXml( ci::XmlTree &xml );
    
	virtual bool getLoop() const { return mIsLooping; }
	virtual void setLoop(bool loop) { mIsLooping = loop; mMovie->setLoop(mIsLooping); }
	virtual void seekToStart() { mMovie->seekToStart(); }
	virtual void stop() { mMovie->stop(); }
	virtual void play() { mMovie->play(); }
    
protected:
	XMovie() : mIsLooping(false) {}
    
	bool mIsLooping;
    ci::gl::TextureRef mFrameTexture;
	ci::qtime::MovieGlRef mMovie;
};

}