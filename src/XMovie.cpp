#include "cinder/app/App.h"
#include "cinder/ImageIO.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "XMovie.h"

using namespace ci;
using namespace xui;

XMovieRef XMovie::create()
{
    XMovieRef ref = XMovieRef( new XMovie() );
    return ref;
}

XMovieRef XMovie::create( ci::XmlTree &xml )
{
	XMovieRef ref = XMovie::create();
	ref->loadXml( xml );

	return ref;
}

void XMovie::draw(float opacity)
{
    // Matrix is already applied so we can draw at origin
    gl::color( mColor * ColorA(1.0f, 1.0f, 1.0f, mOpacity * opacity) );
	if( mFrameTexture )
		gl::draw( mFrameTexture, Rectf(vec2(0,0), vec2(mWidth, mHeight)) );
    
    // and then any children will be draw after this
}

void XMovie::update( double elapsedSeconds )
{
	if( mMovie )
		mFrameTexture = mMovie->getTexture();
}

void XMovie::loadXml( ci::XmlTree &xml )
{
	XRect::loadXml(xml);

	// get/set properties from xml
	if ( xml.hasAttribute( "path" ) )
	{
		mMovie = qtime::MovieGl::create( app::getAssetPath( xml.getAttributeValue<std::string>( "path" ) ) );
		mMovie->play();
	}
}