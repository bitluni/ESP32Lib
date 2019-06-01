# bitluni's ESP32Lib
ESP32Lib is a collection features for the ESP32 including highest performance VGA graphics (sprites, 3D), sound and game controllers packed in an Arduino library.
Check out https://youtube.com/bitlunislab for project updates

If you found it useful please consider supporting my work on

Patreon https://patreon.com/bitluni
Paypal https://paypal.me/bitluni

# Acknowledgements
Thanks to Ivan Grokhotkov & Jeroen Domburg (aka Sprite_tm) for their great work on I2S revealing some nitty-gritty details and quirks of the ESP32.
Special thanks to Fabrizio Di Vittorio for the inpiration to look deeper into 8Bit modes enabling higher resolutions. He developed the FabGL library simultaneously.

# License
bitluni 2019
Creative Commons Attribution ShareAlike 4.0
https://creativecommons.org/licenses/by-sa/4.0/

If you need another license, please feel free to contact me

# Documentation

## Installation

This library only supports the ESP32.
I be able to install the ESP32 features in the board manager you need to add an additional Boards Manager URL in the Preferences (File -> Preferences)
```
https://dl.espressif.com/dl/package_esp32_index.json
```
The ESP32Lib can be found in the Library Manager (Sketch -> Include Library -> Manage Libaries)
To be able to use the library featues the main header needs to included in the sketch
```cpp
#include <ESP32Lib.h>
```

## VGA Features

ESP32Lib implements VGA output over I²S.  
The highest possible resolution with this library is 800x600.
Many common resolutions like 320x240 are preconfigured und can be used without any effort.
Two color depths are available. 14Bit R5G5B4 and 3Bit(8 color) R1G1B1 for convenience and memory savings.

To simplify things you can find boards specially designed to work with this library in my shop:
https://www.tindie.com/stores/bitluni/
Any purchase supports the further development. Thanks!

### Predefined Pin Configurations
A simplified way to configure the pins for shields from my tindie store is using the default predefined configurations VGAv01, VGABlackEdition, VGAWhiteEdition, PicoVGA like this:
```cpp
vga.init(vga.MODE320x200, vga.VGABlackEdition); 
```

### Pin configuration

An VGA cable can be used to connect to the ESP32
The connector pins can be found here: https://en.wikipedia.org/wiki/VGA_connector
The 3Bit modes are very easy to set up. You can connect 
the Ground, Red, Green, Blue, hSync and vSync to output pins of the ESP32.

![3Bit color setup](/Documentation/schematic3bit.png)

The 14Bit mode require a resistor ladder to be set up for each color (DAC) as shown here

![14Bit color setup](/Documentation/schematic.png)

There are limitation on which the VGA output an DACs can work:

I/O GPIO pads are 0-19, 21-23, 25-27, 32-39
Input/Output GPIOs are 0-19, 21-23, 25-27, 32-33. 
GPIO pads 34-39 are input-only.

Beware of pin 0. It selects the boot mode.
It's not suitable for color channels and might cause problems as sync signals as well.

Pin 5 is often tied to the LED but can be used as hSync if you don't need the LED.
I²C is on 21 (SDA), 22(SCL)
DACs are 25, 26
ins 1(TX), 3(RX) sould only be used if really don't need to communicate anymore.

Here is an overview for your convenience:
(0), 2, 4, (5), 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 27, 32, 33

### Usage

There are 4 diffent VGA Drivers **VGA3Bit**, **VGA3BitI**, **VGA14Bit** and **VGA14BitI**.
VGA3Bit, VGA14Bit are the high performance drivers that don't need any CPU time to
serve the VGA. However the VGA3Bit driver takes twice the memory compared to VGA3BitI.
The high performace drivers work the best with the WiFi features. The other driver might
cause errors. WiFi should connect first before VGA3BitI and VGA14BitI is initialized.
The *I* drivers are using an interrupt to feed the pixels to the I²S. This feature can be used for realtime outputs (Check the **VGANoFramebuffer** example).
An instance of the driver has to be created. The optional parameter (only for 14 bit versions) is the I²S bus to be used. If no parameter is given 1 is used by default to keep I²S0 free for audio output.
```cpp
VGA14Bit vga(1);
```
Creating the instance does nothing until the init method is called once. The driver will initialize with one frame buffer by default (that's the memory where the pixels are stored).
Showing animated using only one frame buffer will cause flickering since the frame will be displayed while it is redrawn. To have smooth animations double buffering is recommended.
A second - the back buffer - is used to paint the graphics in the back ground. When the rendering is finished the front and back buffer are flipped. The disadvantage of the second buffer is
the doubled memory requirements. 320x240 will no longer work (320x200 will).
**Double buffering** is enabled using
```cpp
vga.setFrameBufferCount(2);
```
before the init method is called (not calling it will result in a single buffer init)
```cpp
vga.init(vga.MODE320x200, redPins, greenPins, bluePins, hsyncPin, vsyncPin);
```

The R, G and B pins are passed as arrays for the 14Bit driver and as single integers for the 3Bit version. Please try the examples
The following modes are predefined:
- MODE320x480
- MODE320x240
- MODE320x120
- MODE320x400
- MODE320x200
- MODE360x400
- MODE360x200
- MODE360x350
- MODE360x175
- MODE320x350
- MODE320x175
- MODE400x300
- MODE400x150
- MODE400x100
- MODE200x150
- MODE500x480
- MODE500x240
- MODE800x600
- MODE720x400
- MODE720x350
- MODE640x480
- MODE640x400
- MODE800x600

These native modes require a too high pixel clock but can be used as a base to create a custom resolution. Please check out the **VGACustomResolution** example:
- MODE1280x1024
- MODE1280x960
- MODE1280x800
- MODE1024x768

### 2D features
The vga instance implements several drawing methods that can be seen in the **VGA2DFeatures** example.
A complete list will be available in an API documentation soon...

## Converting 3D Meshes
The Utilities folder provides a convenient [StlConverter](https://htmlpreview.github.io/?https://github.com/bitluni/ESP32Lib/blob/master/Utilities/StlConverter.html) that you can use directly from the browser. No worries, your files are not uploaded.
Make sure your STL is low poly. The 3D example use a model with < 5000 triangles.


## Converting Sprites and Images
The Utilities folder provides a convenient [SpriteEditor](https://htmlpreview.github.io/?https://github.com/bitluni/ESP32Lib/blob/master/Utilities/SpriteEditor.html) that you can use directly from the browser. No worries, your files are not uploaded.
The correct pixel format for VGA14Bit and VGA3Bit is R5G5B4A2 (that will be improved in future). You can import PNG files to use transparency.
Each sprite has the origin in the center by default. You can modify it by changing the x/y values of the first point definition. Clicking on the image creates additional points that can be used for other purpouses like hit boxes etc.