#pragma once

#include "cinder/Function.h"
#include "XNode.h"

namespace xui {

class XScene : public XNode {
    
public:
    static XSceneRef create();
	static XSceneRef create(std::string xmlAsset);
    virtual ~XScene();    

    bool touchesBegan( ci::app::TouchEvent event );
    bool touchesMoved( ci::app::TouchEvent event );
    bool touchesEnded( ci::app::TouchEvent event );

	virtual void dispatchStateEvent( const std::string& event );

	template<typename T>
    ci::CallbackId registerEvent( T *obj, void (T::*callback)(std::string) )
	{
		return mCbStateEvent.registerCb(std::bind1st(std::mem_fun(callback), obj));
	}    
    
protected:
    XScene();

	void loadXmlAsset(std::string xmlAsset);
	void loadXml(ci::XmlTree &xml);

	ci::CallbackMgr<void(std::string)> mCbStateEvent;
};

}