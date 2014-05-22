XUI
================

XML based UI System for Cinder

#####Features:
* Scene graph system for nodes.
* Dynamic reloading of changed xml assets.
* Basic controls (Rect, Text, Image).
* State/Event system with inheritance for states.
* Touch handling.

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