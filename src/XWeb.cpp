#if defined(CINDER_AWESOMIUM)
#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIO.h"
#include "XWeb.h"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"

#include "CinderAwesomium.h"

using namespace ci;
using namespace xui;

// fix for building Awesomium in OSX
#if defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
Awesomium::DataSource::~DataSource()
{
}
#endif

// Datasource for loading xml into Awesomium
class XmlDataSource : public Awesomium::DataSource 
{
	std::string xmlContent;
public:
	XmlDataSource(std::string& xmlStr) : xmlContent(xmlStr) {}
	virtual ~XmlDataSource() {}

	virtual void OnRequest(int request_id, const Awesomium::ResourceRequest& request, const Awesomium::WebString& path) 
	{
		if (path == Awesomium::WSLit("index.html"))
			SendResponse(request_id, xmlContent.size(), (unsigned char*)xmlContent.c_str(), Awesomium::WSLit("text/html"));
	}
};

XWeb::XWeb() : mWebViewPtr(NULL)
{
	// create our web session and web view
	mWebSessionPtr = Awesomium::WebCore::instance()->CreateWebSession(Awesomium::WSLit(""), Awesomium::WebPreferences());
}

XWeb::~XWeb()
{
	mWebSessionPtr->Release();
	if (mWebViewPtr != NULL)
		mWebViewPtr->Destroy();
}

XWebRef XWeb::create()
{
    XWebRef ref = XWebRef( new XWeb() );
    return ref;
}

XWebRef XWeb::create( ci::XmlTree &xml )
{
	XWebRef ref = XWeb::create();
	ref->loadXml( xml );
	return ref;
}

void XWeb::draw(float opacity)
{
    // Matrix is already applied so we can draw at origin
    gl::color( mColor * ColorA(1.0f, 1.0f, 1.0f, mOpacity * opacity) );
	if (mWebTexture)
	{
		gl::enableAlphaBlending(true);
		gl::draw(mWebTexture);
	}
    
    // and then any children will be draw after this
}

void XWeb::update(double elapsedSeconds)
{
	// create or update our OpenGL Texture from the webview
	if (ph::awesomium::isDirty(mWebViewPtr))
	{
		try {
			// set texture filter to NEAREST if you don't intend to transform (scale, rotate) it
			gl::Texture::Format fmt;
			fmt.setMagFilter(GL_NEAREST);

			// get the texture using a handy conversion function
			mWebTexture = ph::awesomium::toTexture(mWebViewPtr, fmt);
		}
		catch (const std::exception &e) {
			app::console() << e.what() << std::endl;
		}
	}
}

void XWeb::loadXml( ci::XmlTree &xml )
{
	XRect::loadXml(xml);

	if (xml.hasAttribute("url"))
	{
		// if we have a url, use that as our content
		mWebViewPtr = Awesomium::WebCore::instance()->CreateWebView((int)mWidth, (int)mHeight, mWebSessionPtr);
		mWebViewPtr->LoadURL(Awesomium::WebURL(Awesomium::WSLit(xml.getAttributeValue<std::string>("url").c_str())));
	}
	else
	{
		// if we have an xml child, load that as Awesomium content
		std::shared_ptr<rapidxml::xml_document<char> > xmlDoc = xml.createRapidXmlDoc();
		std::stringstream ss;
		ss << *xmlDoc;
		std::string xmlStr = ss.str();

		// create a datasource to load up our xml
		XmlDataSource* xmlDataSource = new XmlDataSource(xmlStr);
		mWebSessionPtr->AddDataSource(Awesomium::WSLit("MyApplication"), xmlDataSource);

		// create the web view for this item
		mWebViewPtr = Awesomium::WebCore::instance()->CreateWebView((int)mWidth, (int)mHeight, mWebSessionPtr);
		mWebViewPtr->LoadURL(Awesomium::WebURL(Awesomium::WSLit("asset://MyApplication/index.html")));
	}


	mWebViewPtr->Focus();
	mWebViewPtr->SetTransparent(true);
}

#endif