#include "cinder/Utilities.h"

#include "XUI.h"
#include "XScript.h"
#include "XScene.h"
#include "XNode.h"
#include "XRect.h"
#include "XText.h"
#include "XImage.h"
#include "XMovie.h"
#if defined(CINDER_AWESOMIUM)
#include "XWeb.h"
#endif
#include "XSVG.h"
#include "XCarousel.h"
#include "XControl.h"

#include "cinder/gl/gl.h"

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
  static void setScaleX(XNode* node, float value) { node->setScale(vec2(value, node->getScale().y)); }
  static void setScaleY(XNode* node, float value) { node->setScale(vec2(node->getScale().x, value)); }
  static void setOpacity(XNode* node, float value) { node->setOpacity(value); }
};

struct XTextHelper
{
	static std::string getText(XText const* node) { return node->getText(); }
	static void setText(XText* node, std::string value) { node->setText(value); }
};
    
int XScript::panic( lua_State *L )
{
    XUI::getInstance()->consoleOut("Lua panic ocurred! : " + std::string(lua_tostring(L, -1)));
    return 0;
}

void lua_print(std::string msg) 
{
    XUI::getInstance()->consoleOut("LUA: " + msg);
}
    
void lua_uniform_float(ci::gl::GlslProg* prog, const char* name, float value) 
{
	prog->uniform(name, value);
}
void lua_uniform_bool(ci::gl::GlslProg* prog, const char* name, bool value) 
{
	prog->uniform(name, value);
}
void lua_uniform_int(ci::gl::GlslProg* prog, const char* name, int value)
{
	prog->uniform(name, value);
}
    
void pushModalScene(std::string xuiScene)
{
    // push a scene onto the stack
    XUI::getInstance()->pushScene(xuiScene);
}
    
void popModalScene(std::string luaCommand)
{
    // pop a scene off of the stack
    XUI::getInstance()->popScene(luaCommand);
}
    
void transitionToScene(std::string scene, std::string transitionType, float transitionDuration)
{
    // pop a scene off of the stack
    XUI::getInstance()->transitionToXUIScene(scene, transitionType, transitionDuration);
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
		.addFunction("uniform_float", lua_uniform_float)
		.addFunction("uniform_bool", lua_uniform_bool)
		.addFunction("uniform_int", lua_uniform_int)
        .addFunction("pushModalScene", pushModalScene)
        .addFunction("popModalScene", popModalScene)
        .addFunction("transitionToScene", transitionToScene)
		.beginClass<ci::gl::GlslProg>("GlslProg")
			//.addConstructor <void (*) (const char *vertexShader, const char *fragmentShader)> ()
			.addFunction ("bind", &ci::gl::GlslProg::bind)
			//.addStaticFunction ("unbind", &ci::gl::GlslProg::unbind)
		.endClass()
		.beginClass<ci::ColorA>("Color")
			.addConstructor <void (*) (float, float, float)> ()
			.addConstructor <void (*) (float, float, float, float)> ()
		.endClass()
		.beginClass<XNode>("XNode")
			.addProperty("visible", &XNode::getVisible, &XNode::setVisible)
			.addProperty("enabled", &XNode::getEnabled, &XNode::setEnabled)
			.addProperty("x", &XNode::getX, &XNode::setX)
			.addProperty("y", &XNode::getY, &XNode::setY)
			.addProperty("rotation", &XNode::getRotation, &XNode::setRotation)
			.addProperty("scaleX", &XNodeHelper::getScaleX, &XNodeHelper::setScaleX)
			.addProperty("scaleY", &XNodeHelper::getScaleY, &XNodeHelper::setScaleY)
			.addProperty("opacity", &XNode::getOpacity, &XNode::setOpacity)
            .addFunction("setState", &XNode::setState)
            .addFunction("playSound", &XNode::playSound)
            .addFunction("delay", &XNode::luaDelay)

			// rect
			.addProperty("width", &XNode::getWidth, &XNode::setWidth)
			.addProperty("height", &XNode::getHeight, &XNode::setHeight)
			.addProperty("color", &XNode::getColor, &XNode::setColor)
            .addProperty("strokeColor", &XNode::getStrokeColor, &XNode::setStrokeColor)
            .addProperty("cornerRadius", &XNode::getCornerRadius, &XNode::setCornerRadius)
			.addProperty("mask", &XNode::getMask, &XNode::setMask)
			.addProperty("panEnabled", &XNode::getPanEnabled, &XNode::setPanEnabled)
			.addProperty("scaleEnabled", &XNode::getScaleEnabled, &XNode::setScaleEnabled)
			.addProperty("rotateEnabled", &XNode::getRotateEnabled, &XNode::setRotateEnabled)

			// text
			.addProperty("text", &XNode::getText, &XNode::setText)
			.addProperty("textWidth", &XNode::getTextWidth)
			.addProperty("textHeight", &XNode::getTextHeight)

			// movie
			.addProperty("loop", &XNode::getLoop, &XNode::setLoop)
			.addFunction("seekToStart", &XNode::seekToStart)
			.addFunction("play", &XNode::play)
			.addFunction("stop", &XNode::stop)

			// carousel
			.addProperty("carouselEnabled", &XNode::getCarouselEnabled, &XNode::setCarouselEnabled)

        .endClass()
		.deriveClass <XRect, XNode> ("XRect")
		.endClass()
		.deriveClass <XMovie, XRect>("XMovie")
		.endClass()
		.deriveClass <XText, XRect> ("XText")
		.endClass()
#if defined(CINDER_AWESOMIUM)
		.deriveClass <XWeb, XRect>("XWeb")
		.endClass()
#endif
		.deriveClass <XCarousel, XRect>("XCarousel")
		.endClass()
        .deriveClass <XCarousel, XRect>("XControl")
        .endClass();

	luabridge::push(mState, &(*(node)));
	lua_setglobal(mState, "this");
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
        XUI::getInstance()->consoleOut(mLastErrorString);
        return;
    }
    else mErrors = false;
        
    // run the string
    if( lua_pcall( mState, 0, LUA_MULTRET, 0 ) != 0 ) {
        mErrors = true;
        mLastErrorString = getErrorMessage();
        XUI::getInstance()->consoleOut(mLastErrorString);
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
        XUI::getInstance()->consoleOut("Lua Error trying to call " + function + " : " + e.what());
	}
}

void XScript::LuaGlobalBindAllChildren(XNode* node)
{
	// create giant list of all children
	std::vector<XNode*> allChildrenList;
	std::function<void(XNode*)> recurseBuildListFn;
	recurseBuildListFn = [&allChildrenList, &recurseBuildListFn](XNode* node) {
		for (int i = 0; i < node->getNumChildren(); i++)
		{
			allChildrenList.push_back(&(*node->getChildAt(i)));
			recurseBuildListFn(&(*node->getChildAt(i)));
		}
	};
	recurseBuildListFn(node);

	// go through all children and lua bind every other node
	for (unsigned int i = 0; i < allChildrenList.size(); i++)
	{
		for (unsigned int j = 0; j < allChildrenList.size(); j++)
		{
			if (i != j && 
				allChildrenList[j]->getId() != "" &&
				allChildrenList[i]->getScript())
			{
				XNode* bindNode = &(*(allChildrenList[j]));
				luabridge::push(allChildrenList[i]->getScript()->getState(), bindNode);
				lua_setglobal(allChildrenList[i]->getScript()->getState(), bindNode->getId().c_str());
			}
		}
	}
    
    // finally bind all children to the top node too
    for (unsigned int j = 0; j < allChildrenList.size(); j++)
    {
        if (allChildrenList[j]->getId() != "" &&
            node->getScript())
        {
            XNode* bindNode = &(*(allChildrenList[j]));
            luabridge::push(node->getScript()->getState(), bindNode);
            lua_setglobal(node->getScript()->getState(), bindNode->getId().c_str());
        }
    }

}

}