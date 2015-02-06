#include "cinder/app/App.h"
#include "cinder/ImageIO.h"
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
		gl::draw( mFrameTexture, Rectf(Vec2f(0,0), Vec2f(mWidth, mHeight)) );
    
    // and then any children will be draw after this
}

void XMovie::update( double elapsedSeconds )
{
	if( mMovie )
		mFrameTexture = mMovie->getTexture();
}

void XMovie::loadXml( ci::XmlTree &xml )
{
	std::string texture;

	// get/set properties from xml
	if ( xml.hasAttribute( "path" ) )
	{
		mMovie = qtime::MovieGl::create( app::getAssetPath( xml.getAttributeValue<std::string>( "path" ) ) );
		//mMovie->setLoop();
		//mMovie->play();
	}

	XRect::loadXml( xml );
}