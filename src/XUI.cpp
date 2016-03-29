//
//  XUI.cpp
//

#include "cinder/Timeline.h"
#include "cinder/gl/gl.h"

#include "boost/bind.hpp"

// XUI
#include "XUI.h"
#include "XAssetManager.h"
#include "XScene.h"
#include "XRect.h"

#if defined(CINDER_AWESOMIUM)
#include "CinderAwesomium.h"
#endif

using namespace ci;
using namespace ci::app;
using namespace std;

namespace xui {

XUI* XUI::sInstance = NULL;


XUIRef XUI::create( XUI::Settings settings )
{
    sInstance = new XUI(settings);
    return XUIRef(sInstance);
}

XUI::XUI( Settings settings )
    : mSettings( settings ), mScenePopCount( 0 ), mPopLuaCommand( "" ), mTransitionType( SceneTransitionType::NONE ),
    mTransitionAlpha( 0.0f ), mTransitionDuration( 0.0f )
{
    if( mSettings.getWindow() )
        connectWindow( mSettings.getWindow() );
    mVisible = true;
    
#if defined(CINDER_AWESOMIUM)
    // if we don't have a webcore singleton, go ahead and create a global one
    if (Awesomium::WebCore::instance() == NULL)
    {
        // set Awesomium logging to verbose
        Awesomium::WebConfig cnf;
        cnf.log_level = Awesomium::kLogLevel_Verbose;
        cnf.remote_debugging_host = Awesomium::WSLit("127.0.0.1");
        cnf.remote_debugging_port = 1337;
#if defined( CINDER_MAC )
        // TODO: app path needs to be hardcoded here
        std::string frameworkPath = (getAppPath() / "UITest.app/Contents" / "MacOS").string();
        cnf.package_path = Awesomium::WebString::CreateFromUTF8(frameworkPath.c_str(), frameworkPath.size());
#endif
        
        // initialize the Awesomium web engine
        Awesomium::WebCore::Initialize(cnf);
    }
#endif
}

void XUI::connectWindow( app::WindowRef window )
{
    app::App::get()->getSignalUpdate().connect( std::bind( &XUI::update, this ) );
    app::App::get()->getSignalCleanup().connect( std::bind( &XUI::shutdown, this ) );
    
    window->getSignalDraw().connect( std::bind( &XUI::draw, this ) );
    window->getSignalKeyDown().connect(boost::bind(&XUI::keyDown, this, _1));
    window->getSignalKeyUp().connect(boost::bind(&XUI::keyUp, this, _1));
    window->getSignalMouseMove().connect(boost::bind(&XUI::mouseMove, this, _1));
    window->getSignalMouseDown().connect(boost::bind(&XUI::mouseDown, this, _1));
    window->getSignalMouseDrag().connect(boost::bind(&XUI::mouseDrag, this, _1));
    window->getSignalMouseUp().connect(boost::bind(&XUI::mouseUp, this, _1));
    window->getSignalMouseWheel().connect(boost::bind(&XUI::mouseWheel, this, _1));
    
    window->getSignalTouchesBegan().connect(boost::bind(&XUI::touchesBegan, this, _1));
    window->getSignalTouchesMoved().connect(boost::bind(&XUI::touchesMoved, this, _1));
    window->getSignalTouchesEnded().connect(boost::bind(&XUI::touchesEnded, this, _1));
    
    window->getSignalFileDrop().connect(boost::bind(&XUI::fileDrop, this, _1));
    window->getSignalResize().connect(boost::bind(&XUI::resize, this));
}
    
void XUI::loadXUIAsset( std::string xuiAsset, std::function<void(XSceneRef)> callback )
{
    // setup asset manager to watch this asset and reload it when it changes
    XAssetManager::load( xuiAsset, [this, xuiAsset, callback](DataSourceRef dataSource)
    {
        XSceneRef sceneRef;
        std::string xuiString;
        xuiString.assign((char*)(dataSource->getBuffer()->getData()), dataSource->getBuffer()->getSize());
        
        // send event that we are loading content
        mCbSceneLoadTryEvent.call( SceneLoadEventResponse(xuiAsset, xuiString) );
        
        // try to load this asset
        try {
            sceneRef = XScene::createFromXmlString(xuiString);
            //sceneRef->registerEvent(this, &UITestApp::onSceneEvent);
        }
        catch (ci::XmlTree::Exception &boom) {
            mCbSceneLoadFailureEvent.call( SceneLoadFailureEventResponse( "XML Tree Error: " + std::string(boom.what()), -1));
            return;
        }
        catch (XUIParseError &e)
        {
            // count all occurrence in between start of string
            int charCount = e.where();
            size_t line = std::count(xuiString.begin(), xuiString.begin() + charCount, '\n');
            
            mCbSceneLoadFailureEvent.call( SceneLoadFailureEventResponse( "XML Parse Error: " + std::string(e.what()), line-1));
            return;
        }
        catch (cinder::app::AssetLoadExc e)
        {
            mCbSceneLoadFailureEvent.call( SceneLoadFailureEventResponse( "Error Loading Asset: " + std::string(e.what()), -1));
            return;
        }
        catch (std::exception e) {
            mCbSceneLoadFailureEvent.call( SceneLoadFailureEventResponse( "Error Loading XUI: " + std::string(e.what()), -1));
            return;
        }
        catch (...) {
            mCbSceneLoadFailureEvent.call( SceneLoadFailureEventResponse( "Unknown Error!!", -1));
            return;
        }
        
        // make callback after loading this scene
        callback(sceneRef);
        
        // send event that we are loading content
        mCbSceneLoadSuccessEvent.call( SceneLoadEventResponse(xuiAsset, xuiString) );
    } );
}

void XUI::update()
{
#if defined(CINDER_AWESOMIUM)
    // update the Awesomium engine
    Awesomium::WebCore::instance()->Update();
#endif
    
    if( !mVisible ) return;
    
    // pop of any scenes waiting to be popped
    while (mScenePopCount > 0 && mSceneStack.size() > 1)
    {
        mSceneStack.pop_front();
        --mScenePopCount;
        
        if (mPopLuaCommand != "")
            mSceneStack.front()->getScript()->loadString(mPopLuaCommand);
    }
    mScenePopCount = 0;
    mPopLuaCommand = "";
    
    for (std::deque<XSceneRef>::iterator it = mSceneStack.begin(); it < mSceneStack.end(); ++it)
        (*it)->deepUpdate(app::getElapsedSeconds());
}

void XUI::draw()
{
    if( !mVisible ) return;
    
    gl::clear(Color(0, 0, 0));
    
    // draw the deque in reverse order !!!
    for (std::deque<XSceneRef>::reverse_iterator it = mSceneStack.rbegin(); it < mSceneStack.rend(); ++it)
    {
        switch (mTransitionType)
        {
            case SceneTransitionType::NONE:
                (*it)->deepDraw(1.0f);
                break;
            case SceneTransitionType::FADE:
                (*it)->deepDraw(1.0f - mTransitionAlpha);
                break;
            case SceneTransitionType::SLIDE_LEFT:
                (*it)->deepDraw(1.0f, vec2(-mTransitionAlpha, 0.0f));
                break;
            case SceneTransitionType::SLIDE_RIGHT:
                (*it)->deepDraw(1.0f, vec2(mTransitionAlpha, 0.0f));
                break;
            case SceneTransitionType::SLIDE_UP:
                (*it)->deepDraw(1.0f, vec2(0.0f, -mTransitionAlpha));
                break;
            case SceneTransitionType::SLIDE_DOWN:
                (*it)->deepDraw(1.0f, vec2(0.0f, mTransitionAlpha));
                break;
        }
    }
    
    // also draw transitioning scene
    switch (mTransitionType)
    {
        case SceneTransitionType::NONE:
            break;
        case SceneTransitionType::FADE:
            mTransitionScene->deepDraw(mTransitionAlpha);
            break;
        case SceneTransitionType::SLIDE_LEFT:
            mTransitionScene->deepDraw(1.0f, vec2(1.0f - mTransitionAlpha, 0.0f));
            break;
        case SceneTransitionType::SLIDE_RIGHT:
            mTransitionScene->deepDraw(1.0f, vec2(-1.0f + mTransitionAlpha, 0.0f));
            break;
        case SceneTransitionType::SLIDE_UP:
            mTransitionScene->deepDraw(1.0f, vec2(0.0f, 1.0f - mTransitionAlpha));
            break;
        case SceneTransitionType::SLIDE_DOWN:
            mTransitionScene->deepDraw(1.0f, vec2(0.0f, -1.0f + mTransitionAlpha));
            break;
    }
}

void XUI::shutdown()
{
}

void XUI::resize()
{
}

void XUI::mouseMove( app::MouseEvent event )
{
    if( !mVisible ) return;
}

void XUI::mouseDown( app::MouseEvent event )
{
    if( !mVisible ) return;
    
    for (std::deque<XSceneRef>::iterator it = mSceneStack.begin(); it < mSceneStack.end(); ++it)
        (*it)->mouseDown(event);
}

void XUI::mouseDrag( app::MouseEvent event )
{
    if( !mVisible ) return;
    
    for (std::deque<XSceneRef>::iterator it = mSceneStack.begin(); it < mSceneStack.end(); ++it)
    {
        (*it)->mouseDrag(event);
        if ((*it)->isModal())
            break;
    }
}

void XUI::mouseUp( app::MouseEvent event )
{
    if( !mVisible ) return;
    
    for (std::deque<XSceneRef>::iterator it = mSceneStack.begin(); it < mSceneStack.end(); ++it)
    {
        (*it)->mouseUp(event);
        if ((*it)->isModal())
            break;

    }
}

void XUI::mouseWheel( app::MouseEvent event )
{
    if( !mVisible ) return;
}

void XUI::touchesBegan(ci::app::TouchEvent event)
{
    if( !mVisible ) return;
    
    for (std::deque<XSceneRef>::iterator it = mSceneStack.begin(); it < mSceneStack.end(); ++it)
    {
        (*it)->touchesBegan(event);
        if ((*it)->isModal())
            break;
    }
}

void XUI::touchesMoved(ci::app::TouchEvent event)
{
    if( !mVisible ) return;
    
    for (std::deque<XSceneRef>::iterator it = mSceneStack.begin(); it < mSceneStack.end(); ++it)
    {
        (*it)->touchesMoved(event);
        if ((*it)->isModal())
            break;
    }
}

void XUI::touchesEnded(ci::app::TouchEvent event)
{
    if( !mVisible ) return;
    
    for (std::deque<XSceneRef>::iterator it = mSceneStack.begin(); it < mSceneStack.end(); ++it)
    {
        (*it)->touchesEnded(event);
        if ((*it)->isModal())
            break;
    }
}

void XUI::keyDown( ci::app::KeyEvent event )
{
    if( !mVisible ) return;
    
}
void XUI::keyUp( ci::app::KeyEvent event )
{
    if( !mVisible ) return;
}

void XUI::fileDrop( ci::app::FileDropEvent event )
{
    std::string extension = event.getFile( 0 ).extension().string();
    if (extension == ".xui")
        loadXUIAsset(event.getFile(0).filename().string(), [this](XSceneRef scene)
        {
            mSceneStack.clear();
            mSceneStack.push_front(scene);
        });
}
    
void XUI::loadScene(std::string xuiScene)
{
    // load and push a scene onto the stack
    loadXUIAsset(xuiScene, [this](XSceneRef scene)
    {
        mSceneStack.clear();
        mSceneStack.push_front(scene);
    });
}
    
void XUI::pushScene(std::string xuiScene)
{
    // load and push a scene onto the stack
    loadXUIAsset(xuiScene, [this](XSceneRef scene)
    {
        mSceneStack.push_front(scene);
    });
}

void XUI::popScene(std::string luaCommand)
{
    // we can't pop a scene off of the stack that is in use
    // instead cue up a pop for the next update cycle
    mScenePopCount++;
    
    // also save a command to execute in the new lua scene
    mPopLuaCommand = luaCommand;
}
    
ivec2 XUI::getViewportSize() const
{
    if (mSceneStack.size() > 0)
        return (*mSceneStack.begin())->getViewportSize();
    else
        return mViewportSize;
}
    
void XUI::transitionToXUIScene( std::string xuiScene, std::string transitionType, float transitionDuration)
{
    // load and push a scene onto the stack
    loadXUIAsset(xuiScene, [this, transitionType, transitionDuration](XSceneRef scene)
    {
        mTransitionScene = scene;
        mTransitionDuration = transitionDuration;
        
        // set transition type
        if (transitionType == "fade")
            mTransitionType = SceneTransitionType::FADE;
        if (transitionType == "left")
            mTransitionType = SceneTransitionType::SLIDE_LEFT;
        if (transitionType == "right")
            mTransitionType = SceneTransitionType::SLIDE_RIGHT;
        if (transitionType == "up")
            mTransitionType = SceneTransitionType::SLIDE_UP;
        if (transitionType == "down")
            mTransitionType = SceneTransitionType::SLIDE_DOWN;
        
        // create animation
        mTransitionAlpha = 0.0f;
        ci::app::timeline().apply( &mTransitionAlpha, 1.0f, transitionDuration, EaseInOutQuad() )
            .finishFn([this]()
            {
                // swap the scenes once transition was over
                mSceneStack.clear();
                mSceneStack.push_front(mTransitionScene);
                mTransitionScene.reset();
                mTransitionAlpha = 0.0f;
                mTransitionType = SceneTransitionType::NONE;
            });
    });
}
    
void XUI::consoleOut(std::string msg)
{
    mCbConsoleOut.call(msg);
}

};