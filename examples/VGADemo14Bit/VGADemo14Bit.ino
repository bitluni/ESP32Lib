//This example shows how to animate graphics on a VGA screen. No backbuffering is used... just try it.
//You need to connect a VGA screen cable and an external DAC (simple R2R does the job) to the pins specified below.
//cc by-sa 4.0 license
//bitluni

#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>
#include <math.h>

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
	//initializing i2s vga (with only one framebuffer)
	vga.init(vga.MODE200x150, redPins, greenPins, bluePins, hsyncPin, vsyncPin);
	//setting the font
	vga.setFont(Font6x8);
}

//the loop is done every frame
void loop()
{
	//setting the text cursor to the lower left corner of the screen
	vga.setCursor(0, vga.yres - 8);
	//setting the text color to white with opaque black background
	vga.setTextColor(vga.RGB(0xffffff), vga.RGBA(0, 0, 0, 255));
	//printing the fps
	vga.print("fps: ");
	static long f = 0;
	vga.print(long((f++ * 1000) / millis()));

	//circle parameters
	float factors[][2] = {{1, 1.1f}, {0.9f, 1.02f}, {1.1, 0.8}};
	int colors[] = {vga.RGB(0xff0000), vga.RGB(0x00ff00), vga.RGB(0x0000ff)};
	//animate them with milliseconds
	float p = millis() * 0.002f;
	for (int i = 0; i < 3; i++)
	{
		//calculate the position
		int x = vga.xres / 2 + sin(p * factors[i][0]) * vga.xres / 3;
		int y = vga.yres / 2 + cos(p * factors[i][1]) * vga.yres / 3;
		//clear the center with a black filled circle
		vga.fillCircle(x, y, 8, 0);
		//draw the circle with the color from the array
		vga.circle(x, y, 10, colors[i]);
	}
	//render the flame effect
	for (int y = 0; y < vga.yres - 9; y++)
		for (int x = 1; x < vga.xres - 1; x++)
		{
			//take the avarage from the surrounding pixels below
			int c0 = vga.get(x, y);
			int c1 = vga.get(x, y + 1);
			int c2 = vga.get(x - 1, y + 1);
			int c3 = vga.get(x + 1, y + 1);
			int r = ((c0 & 0x1f) + (c1 & 0x1f) + ((c2 & 0x1f) + (c3 & 0x1f)) / 2) / 3;
			int g = (((c0 & 0x3e0) + (c1 & 0x3e0) + ((c2 & 0x3e0) + (c3 & 0x3e0)) / 2) / 3) & 0x3e0;
			int b = (((c0 & 0x3c00) + (c1 & 0x3c00) + ((c2 & 0x3c00) + (c3 & 0x3c00)) / 2) / 3) & 0x3c00;
			//draw the new pixel
			vga.dotFast(x, y, r | g | b);
		}
}