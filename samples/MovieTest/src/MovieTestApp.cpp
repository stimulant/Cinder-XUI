#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"

#include "XScene.h"
#include "XRect.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace xui;

class MovieTestApp : public AppNative {
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

void MovieTestApp::prepareSettings( Settings *settings )
{
	settings->enableMultiTouch(true);
}

void MovieTestApp::setup()
{
	// ui setup
	mXSceneRef = XScene::create("movie.xml");
	mXSceneRef->registerEvent( this, &MovieTestApp::onSceneEvent );
}

void MovieTestApp::onSceneEvent( std::string event )
{
	console() << event << endl;
}

void MovieTestApp::update()
{
	mXSceneRef->deepUpdate(app::getElapsedSeconds());
}

void MovieTestApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );

	mXSceneRef->deepDraw();
}

void MovieTestApp::mouseDown( ci::app::MouseEvent event )
{
	mXSceneRef->mouseDown(event);
}

void MovieTestApp::mouseUp( ci::app::MouseEvent event )
{
	mXSceneRef->mouseUp(event);
}

void MovieTestApp::mouseDrag( ci::app::MouseEvent event )
{
	mXSceneRef->mouseDrag(event);
}

void MovieTestApp::touchesBegan( ci::app::TouchEvent event )
{
	mXSceneRef->touchesBegan(event);
}

void MovieTestApp::touchesMoved( ci::app::TouchEvent event )
{
	mXSceneRef->touchesMoved(event);
}

void MovieTestApp::touchesEnded( ci::app::TouchEvent event )
{
	mXSceneRef->touchesEnded(event);
}

CINDER_APP_NATIVE( MovieTestApp, RendererGl )
