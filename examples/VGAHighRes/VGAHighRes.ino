//This example shows a high VGA resolution 3Bit mode
//The VGA3BitI implementation uses the I²S interrupt to transcode a dense frame buffer to the needed
//8Bit/sample. Using the dense frame buffer allows to fit the big frame buffer in memory at the price of
//a lot cpu performance.
//You need to connect a VGA screen cable to the pins specified below.
//cc by-sa 4.0 license
//bitluni

//including the needed header
#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>

//pin configuration
const int redPin = 14;
const int greenPin = 19;
const int bluePin = 27;
const int hsyncPin = 32;
const int vsyncPin = 33;

//VGA Device using an interrupt to unpack the pixels from 4bit to 8bit for the I²S
//This takes some CPU time in the background but is able to fit a frame buffer in the memory
VGA3BitI vga;

///draws the bitluni logo
void bitluni(int x, int y, int s)
{
	vga.fillCircle(x + 2 * s, y + 2 * s, 2 * s, vga.RGB(128, 0, 0));
	vga.fillCircle(x + 22 * s, y + 2 * s, 2 * s, vga.RGB(128, 0, 0));
	vga.fillCircle(x + 2 * s, y + 22 * s, 2 * s, vga.RGB(128, 0, 0));
	vga.fillCircle(x + 22 * s, y + 22 * s, 2 * s, vga.RGB(128, 0, 0));
	vga.fillRect(x, y + 2 * s, 24 * s, 20 * s, vga.RGB(128, 0, 0));
	vga.fillRect(x + 2 * s, y, 20 * s, 24 * s, vga.RGB(128, 0, 0));
	vga.fillCircle(x + 7 * s, y + 4 * s, 2 * s, vga.RGB(255, 255, 255));
	vga.fillCircle(x + 15 * s, y + 6 * s, 2 * s, vga.RGB(255, 255, 255));
	vga.fillCircle(x + 11 * s, y + 16 * s, 6 * s, vga.RGB(255, 255, 255));
	vga.fillCircle(x + 13 * s, y + 16 * s, 6 * s, vga.RGB(255, 255, 255));
	vga.fillCircle(x + 11 * s, y + 16 * s, 2 * s, vga.RGB(128, 0, 0));
	vga.fillCircle(x + 13 * s, y + 16 * s, 2 * s, vga.RGB(128, 0, 0));
	vga.fillRect(x + 11 * s, y + 14 * s, 2 * s, 4 * s, vga.RGB(128, 0, 0));
	vga.fillRect(x + 9 * s, y + 14 * s, 2 * s, 2 * s, vga.RGB(128, 0, 0));
	vga.fillRect(x + 5 * s, y + 4 * s, 4 * s, 12 * s, vga.RGB(255, 255, 255));
	vga.fillRect(x + 9 * s, y + 10 * s, 4 * s, s, vga.RGB(255, 255, 255));
}

void setup()
{
	//initializing the graphics mode
	vga.init(vga.MODE800x600, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
	//setting the font
	vga.setFont(Font6x8);
	//clearing with white background
	vga.clear(vga.RGB(0xffffff));
	//text position
	vga.setCursor(10, 10);
	//black text color no background color
	vga.setTextColor(vga.RGB(0));
	//show the remaining memory
	vga.print("free memory: ");
	vga.print((int)heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
	//draw the logo
	bitluni(150, 60, 20);
}

//mainloop
void loop()
{
}