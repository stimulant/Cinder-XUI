#pragma once
#include "XRect.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/gl/Texture.h"
#include "cinder/qtime/QuickTime.h"

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
    
protected:
	XMovie() {}
    
    ci::gl::Texture mFrameTexture;
	ci::qtime::MovieGlRef mMovie;
};

}