#pragma once
#include "XRect.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/gl/Texture.h"
#include "XScene.h"

namespace xui {

typedef std::shared_ptr<class XControl> XControlRef;

class XControl : public XRect
{
public:
    static XControlRef create();
    static XControlRef create( ci::XmlTree &xml );

	virtual XNode::NodeType getType() { return XNode::NodeTypeControl; }
    
    void update();
    void draw(float opacity = 1.0f);
    
    bool mouseDown( ci::app::MouseEvent event );
    bool mouseUp( ci::app::MouseEvent event );
    bool mouseDrag( ci::app::MouseEvent event );
    
    bool touchesBegan( ci::app::TouchEvent event );
    bool touchesMoved( ci::app::TouchEvent event );
    bool touchesEnded( ci::app::TouchEvent event );

	void loadXml( ci::XmlTree &xml );
    
protected:
	XControl() {}
    
    XSceneRef mScene;
};

}