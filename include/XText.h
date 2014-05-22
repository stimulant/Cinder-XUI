#pragma once
#include "XRect.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Text.h"
#include "cinder/gl/Texture.h"

namespace xui {

typedef std::shared_ptr<class XText> XTextRef;

class XText : public XRect
{
public:
    static XTextRef create();
    static XTextRef create( ci::XmlTree &xml );

	virtual XNode::NodeType getType() { return XNode::NodeTypeText; }
    
    void draw(float opacity = 1.0f);

	void loadXml( ci::XmlTree &xml );
	virtual std::string getText() const { return mTextBox.getText(); }
	virtual void setText( const std::string& text ) { mTextBox.setText( text ); }

	void setProperty( const XNodeStateProperty& prop );
    
protected:
	XText() {}

	ci::TextBox	mTextBox;
	ci::Surface	mTextSurface;
};

}