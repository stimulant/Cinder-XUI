#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/app/RendererGl.h"

#include "XUI.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace xui;

class UITestApp : public App {
public:
	void	setup();
    
	void onSceneLoadTryEvent(XUI::SceneLoadEventResponse response);
	void onSceneLoadSuccessEvent(XUI::SceneLoadEventResponse response);
	void onSceneLoadFailureEvent(XUI::SceneLoadFailureEventResponse response);
	void consoleOut(std::string msg);

	// XUI
	XUIRef mXUI;
};

void UITestApp::setup()
{
	// create XUI and hook up scene load events
	mXUI = XUI::create();
	mXUI->registerSceneLoadTryEvent(this, &UITestApp::onSceneLoadTryEvent);
	mXUI->registerSceneLoadSuccessEvent(this, &UITestApp::onSceneLoadSuccessEvent);
	mXUI->registerSceneLoadFailureEvent(this, &UITestApp::onSceneLoadFailureEvent);
	mXUI->registerConsoleOut(this, &UITestApp::consoleOut);

	// ui setup
	mXUI->loadScene("ui.xml");
}

void UITestApp::consoleOut(std::string msg)
{
	app::console() << msg << std::endl;
}

void UITestApp::onSceneLoadTryEvent(XUI::SceneLoadEventResponse response)
{
	app::console() << "Loading XUI Scene..." << std::endl;
}

void UITestApp::onSceneLoadSuccessEvent(XUI::SceneLoadEventResponse response)
{
	app::console() << "XUI Load Success" << std::endl;
}

void UITestApp::onSceneLoadFailureEvent(XUI::SceneLoadFailureEventResponse response)
{
	// output error and send it to code editor
	app::console() << response.mError << std::endl;
}

CINDER_APP(UITestApp, RendererGl, [&](App::Settings *settings) {
	settings->setFullScreen(false);
	settings->setMultiTouchEnabled(true);
});
