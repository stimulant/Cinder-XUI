//
//  XUI.h
//

#pragma once

#include "cinder/app/App.h"
#include "cinder/gl/Texture.h"
#include "cinder/Utilities.h"
#include "cinder/DataTarget.h"
#include "cinder/Function.h"

#include "XScene.h"

namespace xui {

typedef std::shared_ptr< class XUI > XUIRef;

class XUI
{
    static XUI *sInstance;
    
public:
    
    struct Settings {
    public:
        Settings()
        :  mWindow( ci::app::getWindow() )
        {}
        
        ci::app::WindowRef getWindow(){ return mWindow; }
        void setWindow( ci::app::WindowRef window ){ mWindow = window; }
        Settings& window( ci::app::WindowRef window ){ setWindow( window ); return *this; }
        
    private:
        ci::app::WindowRef  mWindow;
    };
    
    struct SceneLoadEventResponse
    {
        SceneLoadEventResponse(std::string xuiAsset = "",
                               std::string xuiString = "")
        {
            mXuiAsset = xuiAsset;
            mXuiString = xuiString;
        }
        
        std::string mXuiAsset;
        std::string mXuiString;
    };
    
    struct SceneLoadFailureEventResponse
    {
        SceneLoadFailureEventResponse(std::string error = "",
                               unsigned int errorLine = -1)
        {
            mError = error;
            mErrorLine = errorLine;
        }
        
        std::string mError;
        unsigned int mErrorLine;
    };
    
    static XUIRef create( Settings settings = Settings() );
    
    // return singleton instance
    static XUI* getInstance() { return sInstance; }
    
    // register an event to call when a new scene get's loaded successfully or fails
    template<typename T>
    ci::CallbackId registerSceneLoadTryEvent( T *obj, void (T::*callback)(SceneLoadEventResponse) )
    {
        return mCbSceneLoadTryEvent.registerCb(std::bind1st(std::mem_fun(callback), obj));
    }
    template<typename T>
    ci::CallbackId registerSceneLoadSuccessEvent( T *obj, void (T::*callback)(SceneLoadEventResponse) )
    {
        return mCbSceneLoadSuccessEvent.registerCb(std::bind1st(std::mem_fun(callback), obj));
    }
    template<typename T>
    ci::CallbackId registerSceneLoadFailureEvent( T *obj, void (T::*callback)(SceneLoadFailureEventResponse) )
    {
        return mCbSceneLoadFailureEvent.registerCb(std::bind1st(std::mem_fun(callback), obj));
    }
    template<typename T>
    ci::CallbackId registerConsoleOut( T *obj, void (T::*callback)(std::string) )
    {
        return mCbConsoleOut.registerCb(std::bind1st(std::mem_fun(callback), obj));
    }
    
    void setVisible( bool visible = true ){ mVisible = visible; }
    bool isVisible(){ return mVisible; }
    void hide(){ setVisible( false ); }
    void show(){ setVisible( true ); }
    
    void update();
    void draw();
    
    void shutdown();
    void resize();
    
    void mouseMove( ci::app::MouseEvent event );
    void mouseDown( ci::app::MouseEvent event );
    void mouseDrag( ci::app::MouseEvent event );
    void mouseUp( ci::app::MouseEvent event );
    void mouseWheel( ci::app::MouseEvent event );
    
    void touchesBegan(ci::app::TouchEvent event);
    void touchesMoved(ci::app::TouchEvent event);
    void touchesEnded(ci::app::TouchEvent event);

    void keyDown( ci::app::KeyEvent event );
    void keyUp( ci::app::KeyEvent event );
    
    void fileDrop( ci::app::FileDropEvent event );
    
    // load/puch and pop scenes on the scene stack
    void loadScene(std::string xuiScene);
    void pushScene(std::string xuiScene);
    void popScene(std::string luaCommand);
    
    glm::ivec2 getViewportSize() const;
    std::deque<XSceneRef>& getSceneStack() { return mSceneStack; }
    
    void transitionToXUIScene( std::string xuiScene, std::string transitionType, float transitionDuration);
    void consoleOut(std::string msg);
    
protected:
    
    XUI( Settings settings = Settings() );
    
    // load an xui asset as a scene and make a callback when finished
    void loadXUIAsset( std::string xuiAsset, std::function<void(XSceneRef)> callback );
    
    void connectWindow( ci::app::WindowRef window );
    ci::ivec2 transformEventPos(ci::ivec2 pos) const;
    
    std::deque<XSceneRef>       mSceneStack;
    unsigned int                mScenePopCount;
    std::string                 mPopLuaCommand;
    Settings                    mSettings;
    bool                        mVisible;
    ci::ivec2                   mViewportSize;
    
    // Transition
    enum class SceneTransitionType
    {
        NONE,
        FADE,
        SLIDE_LEFT,
        SLIDE_RIGHT,
        SLIDE_UP,
        SLIDE_DOWN
    };
    SceneTransitionType         mTransitionType;
    XSceneRef                   mTransitionScene;
    cinder::Anim<float>         mTransitionAlpha;
    float                       mTransitionDuration;
    
    ci::CallbackMgr<void(SceneLoadEventResponse)> mCbSceneLoadTryEvent;
    ci::CallbackMgr<void(SceneLoadEventResponse)> mCbSceneLoadSuccessEvent;
    ci::CallbackMgr<void(SceneLoadFailureEventResponse)> mCbSceneLoadFailureEvent;
    ci::CallbackMgr<void(std::string)> mCbConsoleOut;
};
    
};