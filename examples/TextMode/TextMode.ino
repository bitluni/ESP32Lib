//This example shows a simple "Hello world!" on a VGA screen.
//You need to connect a VGA screen cable to the pins specified below.
//cc by-sa 4.0 license
//bitluni

#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>

//pin configuration
const int redPin = 14;
const int greenPin = 19;
const int bluePin = 27;
const int hsyncPin = 32;
const int vsyncPin = 33;

//VGA Device
VGATextI vga;

const uint8_t frontColors[] = {0x2,0x0,0x1,0x4,0x1,0x7,0x3};
const uint8_t backColors[] = {0x0,0x7,0x0,0x6,0x7,0x0,0x4};

void setup()
{
	//selecting the font
	//IMPORTANT: for text mode this MUST be done BEFORE init
	//           and font can not be changed after init
	vga.setFont(Font6x8);
	//initializing vga at the specified pins
	vga.init(VGAMode::MODE320x240, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
	vga.setCursor(vga.xres/2-6,vga.yres/2-1);
	//displaying the text
	//default coloring
	vga.println("Hello World!");
	delay(10000);
	//custom coloring
	for(int i = 0; i < 10; i++)
	{
		vga.setFrontGlobalColor(0,0,255);
		vga.setBackGlobalColor(0,0,0);
		delay(500);
		vga.setFrontGlobalColor(255,255,0);
		vga.setBackGlobalColor(0,0,255);
		delay(500);
	}
	delay(2000);
	vga.clear();
}

void loop()
{
	for(int i = 0; i < 10; i++)
	{
		vga.setCursor(random(1,vga.xres-6),random(1,vga.yres-1));
		vga.println("Hello World!");
		delay(250);
	}
	delay(1500);
	vga.clear();
	static int currentpalette = 0;
	vga.frontGlobalColor = frontColors[currentpalette];
	vga.backGlobalColor = backColors[currentpalette];
	currentpalette = (currentpalette + 1)%sizeof(frontColors);
}
