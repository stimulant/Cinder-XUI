XUI
================

XML based UI System for Cinder

__Features:__

* Scene graph system for nodes
* Touch handling.
* [Lua](http://www.lua.org/) Scripting
* Support for quicktime video playback
* Dynamic reloading of changed XUI assets
* Modal scenes (popups)
* Viewing of HTML/Web content via [Awesomium](http://www.awesomium.com/)
* Transitions between scenes (fades and slides)
* Basic controls:
    * Rect
    * Text
    * Image
    * Button
    * Movie
    * Web
    * Image Carousel
    * Control

# Compiling from Source

## Initial Project Setup
1. Install [git](https://confluence.atlassian.com/bitbucket/set-up-git-744723531.html) and [Visual Studio 2013 Community Edition](https://www.visualstudio.com/en-us/news/vs2013-community-vs.aspx) if they are not already installed.
2. Open a new git shell command prompt.
3. Clone this repository to a location on your machine and update submodules:
```c
git clone git@github.com:stimulant/Cinder-XUI.git XUI
cd XUI
git submodule update --init --recursive
```
2. Install Quicktime from [here](http://www.apple.com/quicktime/download/).
3. Open UITest.vcxproj in samples\UITest\vc2013 and hit F5 to build and run.

## Awesomium Setup
(necessary if you want to use the Web Node to view HTML/Web Content)

1. Add the [Cinder-Awesomium](https://github.com/gaborpapp/Cinder-Awesomium) block to your project.
1. Unzip the Awesomium libs from [here](https://app.box.com/s/gs0l1dgwbn0fr26wztixy1gjr4q2v9wo) into the blocks\Awesomium directory (copy the build directory into the Awesomium folder).
2. Make sure your project defines the CINDER_AWESOMIUM preprocessor definition.

## Updating Project

1. Open a new git shell command prompt and go to the XUI directory.
2. Pull latest XUI code from github

```c
git pull
```

2. Update the submodules in case there have been any changes in any of the blocks

```c
git submodule update --recursive
```

3. Rebuild and run your XUI project.

# Reference

XUI has it's own markup language for describing various controls that make up a scene and the LUA scripting language to describe interactions between controls in a XUI scene.  XUI and LUA work very similarly to how HTML and Javascript work on the web to describe a visual representation of a page and the interaction that occurs on that page.  Building a XUI-based application involves creating one or more XUI files that each describe an individual scene.  Scenes can be transitioned between to change between difference states of the application.

## XUI Scenes

In the xml file for a scene you can define a Rect with properties for size, color, position and opacity:

```xml
<Scene>
    <Rect id="window" x="20" y="20" width="240" height="200" color="0xff00ffff" opacity="0.2"/>
<Scene>
```

You can also define Text, Images and Movies (and these can be children of rects and are positioned appropriately):

```xml
<Scene>
    <Rect id="window" x="20" y="20" width="240" height="200" color="0xff00ffff" opacity="0.2">
        <Text x="20" text="blah di blah" size="32" font="Futura Medium.ttf"/>
        <Image id="n" texture="btn1_default.png"/>
        <Movie width="240" height="200" id="movie" path="video.mov"/>
    </Rect>
</Scene>
```

### Scene States

You can provide states for nodes or the scene that are either activated via a button's 'setState' property or via Lua code and these can have target nodes to activate on:

```c
<Scene>
    <State id="showRect">
        <Property target="rect" type="opacity" value="1" time="0.5" ease="InOutQuad"/>
    </State>

    <Rect id="rect" x="60" y="50" width="120" height="120" opacity="0"/>
    
    <!-- pressing this button will trigger the showRect state and show the Rect above -->
    <Button id="showButton" default="showButton.png" press="showButton_press.png" x="60" y="300" setState="showRect"/>
</Scene>
```

States define a target node to act on (identified by id), a type of property to change and optionally a time period to animate it to the new value and a easing type to smooth the animation.  There are a number of possible easing modes:

ease | Description
-----|-------------
None | Easing equation for a simple linear tweening with no easing
InQuad | Easing equation for a quadratic (t^2) ease-in, accelerating from zero velocity
OutQuad | Easing equation for a quadratic (t^2) ease-out, decelerating to zero velocity
InOutQuad | Easing equation for a quadratic (t^2) ease-in/out, accelerating until halfway, then decelerating
InCubic | Easing equation function for a cubic (t^3) ease-in, accelerating from zero velocity
OutCubic | Easing equation for a cubic (t^3) ease-out, decelerating to zero velocity
InOutCubic | Easing equation for a cubic (t^3) ease-in/out, accelerating until halfway, then decelerating
InQuart | Easing equation for a quartic (t^4) ease-in, accelerating from zero velocity 
OutQuart | Easing equation for a quartic (t^4) ease-out, decelerating to zero velocity
InOutQuart | Easing equation for a quartic (t^4) ease-in/out, accelerating until halfway, then decelerating
InQuint | Easing equation function for a quintic (t^5) ease-in, accelerating from zero velocity
OutQuint | Easing equation for a quintic (t^5) ease-out, decelerating to zero velocity
InOutQuint | Easing equation for a quintic (t^5) ease-in/out, accelerating until halfway, then decelerating
InExpo | Easing equation for an exponential (2^t) ease-in, accelerating from zero velocity
OutExpo | Easing equation for an exponential (2^t) ease-out, decelerating from zero velocity
InCirc | Easing equation for a circular (sqrt(1-t^2)) ease-in, accelerating from zero velocity
OutCirc | Easing equation for a circular (sqrt(1-t^2)) ease-out, decelerating from zero velocity
InOutCirc | Easing equation for a circular (sqrt(1-t^2)) ease-in/out, accelerating until halfway, then decelerating
InSine | Easing equation for a sinusoidal (sin(t)) ease-in, accelerating from zero velocity
OutSine | Easing equation for a sinusoidal (sin(t)) ease-out, decelerating from zero velocity
InOutSine | Easing equation for a sinusoidal (sin(t)) ease-in/out, accelerating until halfway, then decelerating
InBack | Easing equation for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) ease-in, accelerating from zero velocity
OutBack | Easing equation for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) ease-out, decelerating from zero velocity
InOutBack | Easing equation for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) ease-in/out, accelerating until halfway, then decelerating
InBounce | Easing equation for a bounce (exponentially decaying parabolic bounce) ease-in, accelerating from zero velocity
OutBounce | Easing equation for a bounce (exponentially decaying parabolic bounce) ease-out, decelerating from zero velocity
InOutBounce | Easing equation for a bounce (exponentially decaying parabolic bounce) ease-in/out, accelerating until halfway, then decelerating

### Modal Scenes

Scenes can also be used as modal popups.  This means that a scene can overlay a previous scene, blocking all input and displaying on top of the previous scene.  In the following example btn2 pushes the modalKeyboard.xui scene on top of the screen when pressed:

```xml
<Button id="btn2" default="images/btn2_default.png" press="images/btn2_pressed.png" x="200" y="40">
  <![CDATA[
    function press()
      pushModalScene("modalKeyboard.xui")
    end
  ]]>
</Button>
```

In order to return from a modal scene, use the popModalScene method (you can also provide a Lua string that will execute in the parent scene after popping:

```xml
<Button id="popButton" default="images/btn2_default.png" press="images/btn2_pressed.png" x="200" y="40">
  <![CDATA[
    function press()
      popModalScene("print('modal scene removed')")
    end
  ]]>
</Button>
```

### Scene Transitions

It is also possible to transition between different scenes.  The new scene file is loaded and transitioned to via a variety of transitions (slides and fades).  In the following example pressing the button causes the destination scene to be loaded and transitioned to over 2 seconds.

```xml
<Button id="btn2" default="images/btn2_default.png" press="images/btn2_pressed.png">
  <![CDATA[
    function press()
      transitionToScene("destination.xui", "fade", 2.0)
    end
  ]]>
</Button>
```

__Transition Types:__

Name | Description
-----|-------------
fade|fade over a given duration
up|slide the new scene up as the old scene is also slid up out of view
down|slide the new scene down as the old scene is also slid down out of view
left|slide the new scene left as the old scene is also slid left out of view
right|slide the new scene right as the old scene is also slid right out of view

### Properties

The scene is also a XUI node and thus has a few properties that can be set directly on it:

__Properties:__

Name | Description
-----|-------------
viewportWidth | width in pixels to view in the current window
viewportHeight | height in pixels to view in the current window
isModal | true if the scene is a modal popup (displays on top of previous scenes and blocks all input to other scenes)

## XUI Controls

### __Rect__

Rect is the base control for all XUI controls and defines a rectangle on screen.  A Rectangle can have an X, Y position (from the upper-left screen corner) and a width and height.  Additionally a background color can be defined as well as opacity.  All controls inherit from Rect and have all of its properties and Lua methods available as well as their own.

Example:

```xml
    <Rect id="window" x="200" y="200" width="240" height="200" color="0xff0000ff" rotate="10" opacity="1.0">
        <![CDATA[
        function update(elapsed)
            print("elapsed: " .. elapsed)
        end

        function mouseDown(x, y)
            print("mouseDown")
        end

        function mouseDrag(x, y)
            print("mouseDrag")
        end

        function mouseUp(x, y)
            print("mouseUp")
        end
        ]]>
    </Rect>
```

__Properties:__

Name | Description
-----|-------------
id | id used to access this control from Lua
x | left offset from parent in pixels
y | top offset from parent in pixels
width | width of rectangle in pixels
height | height of rectangle in pixels
color | background color of rectangle as a hex value containing alpha, red, green and blue components (0xff0000ff is blue)
strokeColor | stroke color to outline rectangle as a hex value containing alpha, red, green and blue components (0xff0000ff is blue)
opacity | normalized opacity of rectangle (0.0 is invisible, 1.0 if fully opaque)
cornerRadius | radius over which to curve the corners (0.0 is completely rectangular)
panEnabled | allow rectangle to be moved with touch
rotateEnabled | allow rectangle to be rotated with two touches
scaleEnabled | allow rectangle to be scaled with two touches

__Lua Callbacks:__

Name | Description
-----|-------------
update | called every time the app updates with elapsed time in seconds
mouseDown | called every time a mouse button is pressed inside of the rectangle (with x, y position)
mouseDrag | called every time a mouse is dragged inside of the rectangle (with x, y position)
mouseUp | called every time a mouse button is released inside of the rectangle (with x, y position)
touchBegan | called every time a touch starts inside of the rectangle (with x, y position)
touchMoved | called every time a touch is moved inside of the rectangle (with x, y position)
touchEnded | called every time a touch is ended inside of the rectangle (with x, y position)

__Lua Methods:__

Name | Description
-----|-------------
delay | execute some lua code after a period of time

### __Image__

Image inherits from Rect and is used to display an image in a rectangular area on screen.

Example:

```xml
    <Image texture="testImage.png"/>
```

__Properties:__

Name | Description
-----|-------------
texture | relative path to jpeg, gif or png image in the assets folder

__Lua Callbacks:__

No additional Lua callbacks.

### __Text__

Text inherits from Rect and is used to display a block of text on screen (text can also be displayed with the Web control if it needs more formatting).

Example:

```xml
    <Text color="0xff000000" size="48" text="This is a test" x="0" y="0" font="fonts/HoboStd.otf"/>
```

__Properties:__

Name | Description
-----|-------------
color | for text controls color controls the color of the text (not the background)
size | point size of text
font | path to an otf or ttf font file in the assets folder to use as the display font
text | text to display

__Lua Callbacks:__

No additional Lua callbacks.

### __Button__

Button inherits from Rect and is used to create an on-screen button control that users can interact with either by clicking on or touching.

Example:

```xml
    <Button id="testButton" default="btn1_default.png" press="btn1_pressed.png" x="0" y="100">
        <![CDATA[
    function press()
        mov:play()
        end
    ]]>
    </Button>
```

__Properties:__

Name | Description
-----|-------------
default | image for default state of button, should be relative path to jpeg, gif or png image in the assets folder
press | image for pressed state of button, should be relative path to jpeg, gif or png image in the assets folder
setState | global state (belonging to scene) to set when this button is pressed
playSound | wave or aiff file in assets to play upon button press

__Lua Callbacks:__

Name | Description
-----|-------------
press | called when a user touches or clicks on this button

### __Movie__

Movie inherits from Rect and is used to create an movie control that can play back QuickTime videos.

Example:

```xml
    <Movie height="200" id="mov" path="test.mov" width="240" x="100" y="200">
        <![CDATA[
        function mouseDown(x, y)
            this.play()
        end

        function mouseUp(x, y)
            this.stop()
        end
        ]]>
   </Movie>
```

__Properties:__

Name | Description
-----|-------------
path | video to play in this control, should be relative path to a QuickTime video in the assets folder
loop | whether the video should loop or not

__Lua Callbacks:__

No additional Lua callbacks.

__Lua Methods:__

Name | Description
-----|-------------
play | plays the video
stop | stops the video
seekToStart | goes to the start of the video

### __Web__

Web inherits from Rect and is used to create web control that can show HTML (and inline CSS) content.  This control has a transparent background by default and can be used to show text and other HTML elements formatted with CSS.  The Web control assumes that all content enclosed inside of it's XML element (<Web>) is valid HTML content.

Example:

```xml
    <Web height="200" width="200" x="20" y="60">
        <html>
            <body>
                <p style="background: blue; color: white;">A new background and font color with inline CSS</p>
                <p style="background: white; color: blue;">A new background and font color with inline CSS</p>
            </body>
        </html>
    </Web>
```

__Properties:__

Name | Description
-----|-------------
url | url to view in this Web node

__Lua Callbacks:__

No additional Lua callbacks.

### __Carousel__

Carousel inherits from Rect and is used to create a control that displays a horizontal set of images that the user can scroll through with touch.  Images are listed as children of the Carousel XML element. Images to the left and right of the currently selected image are auto-alpha'd.

Example:

```xml
    <Carousel x="200" y="300" width="200" height="100" sideImageAlpha="0.3" slideMargin="20">
        <Image texture="color1.png"/>
        <Image texture="color2.png"/>
        <Image texture="color3.png"/>
        <Image texture="color4.png"/>
        <Image texture="color5.png"/>
        <Image texture="color6.png"/>
        <![CDATA[
            function slide(i)
                print("Carousel is now on index " .. i)
        end
        ]]>
    </Carousel>
```

__Properties:__

Name | Description
-----|-------------
sideImageAlpha | amount to fade out left/right side images (0.0 is transparent, 1.0 is fully opaque)
slideMargin | extra margin to add between images

__Lua Callbacks:__

Name | Description
-----|-------------
slide | called when a user changes the focus of the carousel, argument is the index of the new carousel image

### __Control__

The Control node allows you to reference a separate XUI scene as a control in a scene.  This effectively creates an instance of that entire scene such that it can be reused multiple times in the parent scene.  When re-using a block of XUI code this can prove quite helpful.

Example:

```xml
<Scene id="scene"> 
    <Control scene="richTextTest.xui" x="200" y="100"/>
</Scene>
```

__Properties:__

Name | Description
-----|-------------
scene | the scene file to reference

## Using the Lua language

Scripting for XUI is provided via [Lua](http://www.lua.org/).  Since Lua is not valid XML it must be inserted into the XUI file as CDATA.  Lua scripts are scoped to the node that they are inserted in ("this" refers to the scoped node and all other nodes can be referenced by id).  Most properties of nodes can be referenced from Lua (off of "this" or the id) and there are a number of Lua functions that are called implicitly (update, mouseDown, mouseUp, mouseDrag, touchBegan, touchEnded, touchMoved).  Additionally all properties of controls can be set by referring to them by the id.control syntax ("window.x = 20" would set the x position of the control with the window id to 20).  You can also execute Lua after a period of time with the delay function ("this:delay('delayCode()', 2.0)").

```xml
<Scene>
    <Rect id="window" x="200" y="200" width="240" height="200" color="0xff0000ff" rotate="10" opacity="1.0">
        <![CDATA[

        function update(elapsed)
            print("elapsed: " .. elapsed)
        end

        function delayCode()
            print("delayed press")
        end

        function mouseDown(x, y)
            print("mouseDown")
            text.text = "mouseDown"
            this.color = Color(1, 0, 1, 1)
            this:delay("delayCode()", 2.0)
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

The official reference for the Lua programming language lives [here](http://www.lua.org/manual/5.3/).  You can find control specfic documetation for Lua in the section on controls.

# Playing Audio

XUI provides two different ways to play back audio files.  Playback of both aiff and wave files is supported as long as they are in the assets folder.  The first way to play back audio is with the playSound property on a button (XUI will play back this audio file upon button press):

```xml
<Button default="images/btn1_default.png" press="images/btn1_pressed.png" x="40" y="40" playSound="sounds/click_04.wav" />
```

The second way it is possible to play audio is from Lua scripts with the playSound function:

```xml
<Button default="images/btn2_default.png" press="images/btn2_pressed.png" x="200" y="40">
    <![CDATA[
        function press()
            print("press")
            this:playSound("sounds/pop_drip.wav")
        end
    ]]>
</Button>
```


#Credits
Developed at [Stimulant](http://stimulant.com).
Inspired by Simon Geilfus's [AssetManager](https://github.com/simongeilfus/Cinder-AssetManager) and Paul Houx's [SimpleSceneGraph](https://github.com/paulhoux/Cinder-Samples/tree/master/SimpleSceneGraph) blocks among others.

#License
Cinder-XUI is licensed under the MIT License, see LICENSE for more information.