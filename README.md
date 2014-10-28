XUI
================

XML based UI System for Cinder

#####Features:
* Scene graph system for nodes.
* Dynamic reloading of changed xml assets.
* Basic controls (Rect, Text, Image).
* State/Event system with inheritance for states.
* Touch handling.
* Import of [SVG](http://en.wikipedia.org/wiki/Scalable_Vector_Graphics) from Adobe Illustrator
* [Lua](http://www.lua.org/) Scripting

#####Block Depedencies
Currently no dependencies

#####Getting Started
You can load an xml scene file like so:

```c
mXSceneRef = XScene::create("ui.xml");
```

Scenes can be rendered with deepDraw:

```c
mXSceneRef->deepDraw();
```

updated with deepUpdate:

```c
mXSceneRef->deepUpdate(app::getElapsedSeconds());
```

and fed touch events with touchesBegan, touchesMoved and touchesEnded:

```c
mXSceneRef->touchesBegan(event);

mXSceneRef->touchesMoved(event);

mXSceneRef->touchesEnded(event);
```

In the xml file for a scene you can define a Rect with properties for size, color, position and opacity:

```c
<Scene>
    <Rect id="window" x="20" y="20" width="240" height="200" color="0xff00ffff" opacity="0.2"/>
<Scene>
```

You can also define Text and Images (and these can be children of rects and are positioned appropriately):

```c
<Scene>
    <Rect id="window" x="20" y="20" width="240" height="200" color="0xff00ffff" opacity="0.2">
        <Text x="20" text="blah di blah" size="32" font="Futura Medium.ttf"/>
        <Image id="n" texture="btn1_default.png"/>
    </Rect>
</Scene>
```

You can provide states for nodes that are either activated implicitly or via code and these can have target nodes to activate on:

```c
<Scene>
    <Rect x="60" y="50" width="120" height="120" opacity="0">
        <Image id="n" texture="btn1_default.png"/>
        <Image id="p" texture="btn1_pressed.png" opacity="0"/>
        <State id="pressed" event="btn1_pressed">
            <Property target="p" type="opacity" value="1"/>
            <Property target="n" type="opacity" value="0"/>
        </State>
        <State id="released" event="btn1_released">
            <Property target="p" type="opacity" value="0"/>
            <Property target="n" type="opacity" value="1"/>
        </State>
    </Rect>
</Scene>
```

When they are activated in either case they send back their "event" string through an event handler:

```c
mXSceneRef->registerEvent( this, &UITestApp::onSceneEvent );

void UITestApp::onSceneEvent( std::string event )
{
    console() << event << endl;
}
```

Nodes states can also inherit from each other:

```c
<State id="normal">
    <Property target="p" type="opacity" value="0"/>
    <Property target="n" type="opacity" value="1"/>
</State>
<State id="pressed" base="btn1_normal">
    <Property target="p" type="opacity" value="1"/>
</State>
```

#####SVG
It is possible to copy [SVG](http://en.wikipedia.org/wiki/Scalable_Vector_Graphics) into XUI files.  The SVG element behaves like a XUI node (and respects positioning as well as inherited opacity).  With the latest Adobe Creative Cloud it is even possible to copy paste SVG directly from inside illustrator:

```c
<Scene>
    <Rect x="0" y="0" width="600" height="600" opacity="1.0" color="0xff000000">

        <!-- Generator: Adobe Illustrator 17.1.0, SVG Export Plug-In  -->
        <svg version="1.1"
             xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:a="http://ns.adobe.com/AdobeSVGViewerExtensions/3.0/"
             x="0px" y="0px" width="512.5px" height="182.1px" viewBox="0 0 512.5 182.1" enable-background="new 0 0 512.5 182.1"
             xml:space="preserve">
            <defs>
            </defs>
            <g>
                <circle fill="#FFFFFF" cx="63" cy="63" r="57"/>
                <linearGradient id="SVGID_1_" gradientUnits="userSpaceOnUse" x1="126" y1="63" x2="0" y2="63">
                    <stop  offset="0" style="stop-color:#AE2573"/>
                    <stop  offset="0.5" style="stop-color:#6D2077"/>
                    <stop  offset="1" style="stop-color:#500778"/>
                </linearGradient>
                <path fill="url(#SVGID_1_)" d="M63,6c31.5,0,57,25.5,57,57s-25.5,57-57,57S6,94.5,6,63S31.5,6,63,6 M63,0C28.3,0,0,28.3,0,63
                    s28.3,63,63,63s63-28.3,63-63S97.7,0,63,0L63,0z"/>
            </g>
        </svg>

    </Rect>
</Scene>

```

#####LUA Scripting
Scripting for XUI is provided via [Lua](http://www.lua.org/).  Since lua is not valid XML it must be inserted into the XUI file as CDATA.  Lua scripts are scoped to the node that they are inserted in ("this" refers to the scoped node and all other nodes can be referenced by id).  Most properties of nodes can be referenced from lua (off of "this" or the id) and there are a number of lua functions that are called implicitly (update, mouseDown, mouseUp, mouseDrag, touchBegan, touchEnded, touchMoved).

```c
<Scene>
    <Rect id="window" x="200" y="200" width="240" height="200" color="0xff0000ff" rotate="10" opacity="1.0">
        <![CDATA[

        function update(elapsed)
            print("elapsed: " .. elapsed)
        end

        function mouseDown(x, y)
            print("mouseDown")
            text.text = "mouseDown"
            this.color = Color(1, 0, 1, 1)
        end

        function mouseDrag(x, y)
            print("mouseDrag")
            text.text = "mouseDrag"
        end

        function mouseUp(x, y)
            print("mouseUp")
            text.text = "mouseUp"
            this.color = Color(0, 0, 1, 1)
        end

        ]]>

        <Text x="20" id="text" text="" size="32" font="Futura Medium.ttf"/>
    </Rect>
</Scene>
```

#####Credits
Developed at [Stimulant](http://stimulant.com).
Inspired by Simon Geilfus's [AssetManager](https://github.com/simongeilfus/Cinder-AssetManager) and Paul Houx's [SimpleSceneGraph](https://github.com/paulhoux/Cinder-Samples/tree/master/SimpleSceneGraph) blocks among others.

#####License
Cinder-XUI is licensed under the MIT License, see LICENSE for more information.