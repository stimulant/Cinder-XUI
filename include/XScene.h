#pragma once

#include "cinder/Function.h"
#include "XNode.h"

namespace xui {

typedef std::shared_ptr<class XScene> XSceneRef;

/* special exception to throw so we know how many characters in the rror occurred*/
class XUIParseError : public std::exception
{
public:
	XUIParseError(const char *what, int where)
		: m_what(what), m_where(where) {}

	//! Gets human readable description of error.
	//! \return Pointer to null terminated description of the error.
	virtual const char *what() const throw() { return m_what; }
	int where() const { return m_where; }

private:
	const char *m_what;
	int m_where;
};

class XScene : public XNode {
    
public:
    static XSceneRef create();
	static XSceneRef createFromXmlString(std::string xmlString);
	static XSceneRef create(std::string xmlAsset);
    virtual ~XScene();

	virtual void update(double elapsedSeconds);
    virtual void deepDraw(float opacity = 1.0f, glm::vec2 offset = glm::vec2(0, 0));

	bool mouseDown( ci::app::MouseEvent event );
    bool mouseUp( ci::app::MouseEvent event );
	bool mouseDrag( ci::app::MouseEvent event );

    bool touchesBegan( ci::app::TouchEvent event );
    bool touchesMoved( ci::app::TouchEvent event );
    bool touchesEnded( ci::app::TouchEvent event );

	virtual void dispatchStateEvent( const std::string& event );

	template<typename T>
    ci::CallbackId registerEvent( T *obj, void (T::*callback)(std::string) )
	{
		return mCbStateEvent.registerCb(std::bind1st(std::mem_fun(callback), obj));
	}
    
    glm::ivec2 getViewportSize() const { return mViewportSize; }
    glm::ivec2 transformEventPos(glm::ivec2 pos) const;
    bool isModal() const { return mIsModal; }
    
protected:
    XScene();

	void loadXmlAsset(std::string xmlAsset);
	void loadXml(ci::XmlTree &xml);

	ci::CallbackMgr<void(std::string)> mCbStateEvent;
    glm::ivec2 mViewportSize;
    bool mIsModal;
};

}