#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"

#include "XScene.h"
#include "XRect.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace xui;

class UITestApp : public AppNative {
public:
	void	prepareSettings( Settings *settings );
	void	setup();
	void	update();
	void	draw();

	void	touchesBegan( ci::app::TouchEvent event );
	void	touchesMoved( ci::app::TouchEvent event );
	void	touchesEnded( ci::app::TouchEvent event );

	void	mouseDown( ci::app::MouseEvent event );
    void	mouseUp( ci::app::MouseEvent event );
	void	mouseDrag( ci::app::MouseEvent event );
    
	void onSceneEvent( std::string event );
    XSceneRef mXSceneRef;
};

void UITestApp::prepareSettings( Settings *settings )
{
	settings->enableMultiTouch(true);
}

void UITestApp::setup()
{
	// ui setup
	mXSceneRef = XScene::create("ui.xml");
	mXSceneRef->registerEvent( this, &UITestApp::onSceneEvent );
}

void UITestApp::onSceneEvent( std::string event )
{
	console() << event << endl;
}

void UITestApp::update()
{
	mXSceneRef->deepUpdate(app::getElapsedSeconds());
}

void UITestApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );

	mXSceneRef->deepDraw();
}

void UITestApp::mouseDown( ci::app::MouseEvent event )
{
	mXSceneRef->mouseDown(event);
}

void UITestApp::mouseUp( ci::app::MouseEvent event )
{
	mXSceneRef->mouseUp(event);
}

void UITestApp::mouseDrag( ci::app::MouseEvent event )
{
	mXSceneRef->mouseDrag(event);
}

void UITestApp::touchesBegan( ci::app::TouchEvent event )
{
	mXSceneRef->touchesBegan(event);
}

void UITestApp::touchesMoved( ci::app::TouchEvent event )
{
	mXSceneRef->touchesMoved(event);
}

void UITestApp::touchesEnded( ci::app::TouchEvent event )
{
	mXSceneRef->touchesEnded(event);
}

CINDER_APP_NATIVE( UITestApp, RendererGl )
