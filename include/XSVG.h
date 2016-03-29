#pragma once
#include "XRect.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/gl/Texture.h"
#include "cinder/svg/Svg.h"
#include "cinder/gl/GlslProg.h"

namespace xui {

typedef std::shared_ptr<class XSVG> XSVGRef;

class XSVG : public XRect
{
public:
    static XSVGRef create();
    static XSVGRef create( ci::XmlTree &xml );

	virtual XNode::NodeType getType() { return XNode::NodeTypeSVG; }
    
    void update();
    void draw(float opacity);

	void loadXml( ci::XmlTree &xml );
    
protected:
	XSVG();
    
	static ci::gl::GlslProgRef	mSVGShader;
	static bool					mSVGShaderCreated;
    ci::svg::DocRef				mDoc;
};

}