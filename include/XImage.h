#pragma once
#include "XRect.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/gl/Texture.h"

namespace xui {

typedef std::shared_ptr<class XImage> XImageRef;

class XImage : public XRect
{
public:
    static XImageRef create();
    static XImageRef create( ci::XmlTree &xml );

	virtual XNode::NodeType getType() { return XNode::NodeTypeImage; }
    
    void update();
    void draw(float opacity = 1.0f);

	void loadXml( ci::XmlTree &xml );
	void setTexture( ci::gl::TextureRef texture ) { mTexture = texture; }
    
protected:
	XImage() {}
    
    ci::gl::TextureRef mTexture;
};

}