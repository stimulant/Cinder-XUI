#pragma once
#include "XRect.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/gl/Texture.h"

// forward declarations for Awesomium
namespace Awesomium
{
	class WebView;
	class WebSession;
}

namespace xui {

typedef std::shared_ptr<class XWeb> XWebRef;

class XWeb : public XRect
{
	Awesomium::WebSession*	mWebSessionPtr;
	Awesomium::WebView*		mWebViewPtr;

public:
	~XWeb();
    static XWebRef create();
    static XWebRef create( ci::XmlTree &xml );

	virtual XNode::NodeType getType() { return XNode::NodeTypeWeb; }
    
	void update(double elapsedSeconds);
    void draw(float opacity = 1.0f);

	void loadXml( ci::XmlTree &xml );
    
protected:
	XWeb();
    
	ci::gl::TextureRef			mWebTexture;
};

}