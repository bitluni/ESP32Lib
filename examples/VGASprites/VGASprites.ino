//This example shows the three methods of how sprites can be rendered on a VGA screen
//You need to connect a VGA screen cable and an external DAC (simple R2R does the job) to the pins specified below.
//cc by-sa 4.0 license
//bitluni

//include libraries
#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>

//include the sprites converted the SpriteConverter. Check the documentation for further infos.
#include "explosion.h"

//pin configuration
const int redPins[] = {2, 4, 12, 13, 14};
const int greenPins[] = {15, 16, 17, 18, 19};
const int bluePins[] = {21, 22, 23, 27};
const int hsyncPin = 32;
const int vsyncPin = 33;

//VGA Device
VGA14Bit vga;

//initial setup
void setup()
{
	//need double buffering
	vga.setFrameBufferCount(2);
	//initializing i2s vga
	vga.init(vga.MODE200x150, redPins, greenPins, bluePins, hsyncPin, vsyncPin);
	//setting the font
	vga.setFont(Font6x8);
}

//just draw each frame
void loop()
{
	//draw a background
	for (int y = 0; y < vga.yres / 10; y++)
		for (int x = 0; x < vga.xres / 10; x++)
			vga.fillRect(x * 10, y * 10, 10, 10, (x + y) & 1 ? vga.RGB(0, 128, 0) : vga.RGB(0, 0, 128));
	//print some labels
	vga.setCursor(36, 41);
	vga.print("draw   drawMix  drawAdd");
	//there are 20 sprites for the explosion. The second parameter is the index of the sprite.
	//We used the milliseconds to calculate the current index of the animation.
	//the last two parameters is the position. During the conversion of the sprite the origin of each sprite is defined.
	//check the Utilities folder for the converter
	//"draw" draws the sprite opaque ignoring any existing alpha channel
	explosion.draw(vga, (millis() / 50) % 20, vga.xres / 4, vga.yres / 2);
	//"drawMix" uses the alpha channel
	explosion.drawMix(vga, (millis() / 50) % 20, vga.xres / 2, vga.yres / 2);
	//"drawAdd" adds the color components of the back ground and the sprite
	explosion.drawAdd(vga, (millis() / 50) % 20, vga.xres * 3 / 4, vga.yres / 2);
	//swap the frame buffers and show the rendering
	vga.show();
}