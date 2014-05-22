#include "cinder/Utilities.h"

#include "XScript.h"
#include "XNode.h"
#include "XRect.h"
#include "XText.h"
#include "XImage.h"
#include "XSVG.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace xui {

struct XNodeHelper
{
  static float getX(XNode const* node) { return node->getX(); }
  static float getY(XNode const* node) { return node->getY(); }
  static float getRotation(XNode const* node) { return node->getRotation(); }
  static float getScaleX(XNode const* node) { return node->getScale().x; }
  static float getScaleY(XNode const* node) { return node->getScale().y; }
  static float getOpacity(XNode const* node) { return node->getOpacity(); }

  static void setX(XNode* node, float value) { node->setX(value); }
  static void setY(XNode* node, float value) { node->setY(value); }
  static void setRotation(XNode* node, float value) { node->setRotation(value); }
  static void setScaleX(XNode* node, float value) { node->setScale(Vec2f(value, node->getScale().y)); }
  static void setScaleY(XNode* node, float value) { node->setScale(Vec2f(node->getScale().x, value)); }
  static void setOpacity(XNode* node, float value) { node->setOpacity(value); }
};

struct XTextHelper
{
	static std::string getText(XText const* node) { return node->getText(); }
	static void setText(XText* node, std::string value) { node->setText(value); }
};
    
int XScript::panic( lua_State *L )
{
    app::console() << "Lua panic ocurred! : " << lua_tostring(L, -1) << endl;
    app::console() << "Closing state" << endl;
    return 0;
}

void lua_print(std::string msg) 
{
	app::console() << "LUA: " << msg << endl;
}
    
XScript::XScript()
{
	mStopOnErrors	= true;
	mErrors			= false;
        
    mState = luaL_newstate();
    luaL_openlibs( mState );
    lua_atpanic( mState, &panic);
}
XScript::~XScript()
{
    gc();        
	lua_close( mState );
}

void XScript::bindNode(XNode* node)
{
	luabridge::getGlobalNamespace(mState)
		.addFunction("print", lua_print)
		.beginClass<ci::ColorA>("Color")
			.addConstructor <void (*) (float, float, float)> ()
			.addConstructor <void (*) (float, float, float, float)> ()
		.endClass()
		.beginClass<XNode>("XNode")
			.addProperty("x", &XNode::getX, &XNode::setX)
			.addProperty("y", &XNode::getY, &XNode::setY)
			.addProperty("rotation", &XNode::getRotation, &XNode::setRotation)
			.addProperty("scaleX", &XNodeHelper::getScaleX, &XNodeHelper::setScaleX)
			.addProperty("scaleY", &XNodeHelper::getScaleY, &XNodeHelper::setScaleY)
			.addProperty("opacity", &XNode::getOpacity, &XNode::setOpacity)
			.addProperty("width", &XNode::getWidth, &XNode::setWidth)
			.addProperty("height", &XNode::getHeight, &XNode::setHeight)
			.addProperty("color", &XNode::getColor, &XNode::setColor)
			.addProperty("text", &XNode::getText, &XNode::setText)
        .endClass()
		.deriveClass <XRect, XNode> ("XRect")
			//.addProperty("color", &XRect::getColor, &XRect::setColor)
		.endClass()
		.deriveClass <XText, XRect> ("XText")
			//.addProperty("text", &XText::getText, &XText::setText)
		.endClass();

	luabridge::push(mState, &(*(node)));
	lua_setglobal(mState, "this");

	for (int i=0; i < node->getNumChildren(); i++)
	{
		if (node->getChildAt(i)->getId() != "")
		{
			XNode* childNode = &(*(node->getChildAt(i)));
			luabridge::push(mState, childNode);
			lua_setglobal(mState, node->getChildAt(i)->getId().c_str());
		}
	}
}
    
std::string XScript::getErrorMessage()
{
    std::ostringstream errorMessage;
    errorMessage << "Lua run-time error: " << lua_tostring(mState, -1);
    return errorMessage.str();
}
		
std::string XScript::getLastErrors()
{
	return mLastErrorString;
}

void XScript::loadFile( DataSourceRef source )
{
    loadString( ci::loadString( source ) );
}
	
void XScript::loadString( const std::string& script )
{
    gc();
        
    mLastErrorString = "";
        
    // load the string
    if( luaL_loadstring( mState, script.c_str() ) != 0 ) {
        mErrors = true;
        mLastErrorString = getErrorMessage();
        app::console() << mLastErrorString << endl;
        return;
    }
    else mErrors = false;
        
    // run the string
    if( lua_pcall( mState, 0, LUA_MULTRET, 0 ) != 0 ) {
        mErrors = true;
        mLastErrorString = getErrorMessage();
        app::console() << mLastErrorString << endl;
        return;
    }
    else mErrors = false;
                
	call( "setup" );
}
    
    
void XScript::gc()
{
	lua_gc( mState, LUA_GCCOLLECT, 0 );
}

bool XScript::functionExists( const std::string& function ) 
{
	luabridge::LuaRef functionRef = luabridge::getGlobal(mState, function.c_str());
	int type = functionRef.type();
	return functionRef.isFunction();
}

void XScript::call( const std::string& function )
{
	if( !functionExists( function ) || ( mStopOnErrors && mErrors ) )
		return;
	try
	{
		luabridge::LuaRef functionRef = luabridge::getGlobal(mState, function.c_str());
		functionRef();
		mErrors = false;
	}
	catch (luabridge::LuaException const& e) 
	{
		mErrors = true;
		mLastErrorString = e.what();
		ci::app::console() << "Lua Error trying to call " << function << " : " << std::endl << e.what() << std::endl;
	}
}

}