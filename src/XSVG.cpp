#include "cinder/app/App.h"
#include "cinder/ImageIO.h"
#include "XSVG.h"
#include "cinder/gl/gl.h"
#include "cinder/svg/SvgGl.h"
#include "XAssetManager.h"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"

using namespace ci;
using namespace xui;
using namespace svg;

ci::gl::GlslProgRef	XSVG::mSVGShader;
bool				XSVG::mSVGShaderCreated = false;;

#define REGISTER_SHADER(file1, file2, shader) \
XAssetManager::load( file1, file2, [this](DataSourceRef vert,DataSourceRef frag){ \
    try{ \
        shader = gl::GlslProg::create( vert, frag ); \
    } catch( gl::GlslProgCompileExc exc ) { \
		app::console() << exc.what() << std::endl; \
	} \
} );

XSVGRef XSVG::create()
{
    XSVGRef ref = XSVGRef( new XSVG() );
    return ref;
}

XSVGRef XSVG::create( ci::XmlTree &xml )
{
	XSVGRef ref = XSVG::create();
	ref->loadXml( xml );
	return ref;
}

XSVG::XSVG()
{
	if (!mSVGShaderCreated)
	{
		REGISTER_SHADER("svg.vert", "svg.frag", mSVGShader);
		mSVGShaderCreated = true;
	}
}

void XSVG::draw(float opacity)
{
	// set opacity via shader
	mSVGShader->bind();
	mSVGShader->uniform("opacity", opacity * mOpacity);
	
    // draw svg
    gl::draw( *mDoc );

	//mSVGShader.unbind();
}

void XSVG::loadXml( ci::XmlTree &xml )
{
	XRect::loadXml(xml);

	// if we have an svg child, load that
	std::shared_ptr<rapidxml::xml_document<char> > xmlDoc = xml.createRapidXmlDoc();
    std::stringstream ss;
    ss << *xmlDoc;
    std::string svgStr = ss.str();

	std::vector<char> bytesVec(svgStr.begin(), svgStr.end());
	void *bytes = &bytesVec[0];
	unsigned int size = svgStr.size();
	DataSourceRef svgData = DataSourceBuffer::create(Buffer::create(bytes, size));
	mDoc = svg::Doc::create(svgData);
}