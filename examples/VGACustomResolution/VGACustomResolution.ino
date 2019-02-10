//This example shows how a custom VGA resolution can be created for one of the base modes
//You need to connect a VGA screen cable to the pins specified below.
//cc by-sa 2.0 license
//bitluni

//including the needed header
#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>

//pin configuration
const int redPin = 14;
const int greenPin = 19;
const int bluePin = 27;
const int hsyncPin = 0;
const int vsyncPin = 5;

//VGA Device using an interrupt to unpack the pixels from 4bit to 16bit for the I²S
//This takes some CPU time in the background but is able to fit a frame buffer in the memory
VGA3BitI vga;

void setup()
{
	//enabling double buffering
	vga.setFrameBufferCount(2);
	//let's calculate the max horizontal resolution we can get from a given mode
	int maxy = vga.maxXRes(vga.MODE800x600);
	int myMode[13];
	//calculate the parameters for our custom resolution.
	//the y resolution is only scaling integer divisors (yet).
	//if you don't like to let it scale automatically add a fith parameter with the divisor.
	vga.customMode(vga.MODE800x600, maxy, 300, myMode);
	//initializing the graphics mode
	vga.init(myMode, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
	//setting the font
	vga.setFont(Font6x8);
}

///draws a bouncing ball
void ball()
{
	//some basic gravity physics
	static float y = 400;
	static float x = 200;
	static float vx = 10;
	static float vy = 0;
	static unsigned long lastT = 0;
	unsigned long t = millis();
	float dt = (t - lastT) * 0.001f;
	lastT = t;
	const int r = 40;
	int rx = r;
	int ry = r;
	vy += -9.81f * dt * 100;
	x += vx;
	y += vy * dt;
	//check for boundaries and bounce back
	if (y < r && vy < 0)
	{
		vy = 700;
		ry = y;
	}
	if (x < r && vx < 0)
	{
		vx = -vx;
		rx = x;
	}
	if (x >= vga.xres - r && vx > 0)
	{
		vx = -vx;
		rx = vga.xres - x;
	}
	//draw a filled ellipse
	vga.fillEllipse(x, vga.yres - y - 1, rx, ry, vga.RGB(0, 0, 255));
}

//mainloop
void loop()
{
	//draw a background
	for (int y = 0; y * 10 < vga.yres; y++)
		for (int x = 0; x * 10 < vga.xres; x++)
			vga.fillRect(x * 10, y * 10, 10, 10, (x + y) & 1 ? vga.RGB(255, 0, 0) : vga.RGB(255, 255, 255));
	//text position
	vga.setCursor(10, 10);
	//black text color no background color
	vga.setTextColor(vga.RGB(0));
	//show the remaining memory
	vga.print(vga.xres);
	vga.print("x");
	vga.println(vga.yres);
	vga.print("free memory: ");
	vga.print((int)heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
	//draw bouncing ball
	ball();
	//show the backbuffer (only needed when using backbuffering)
	vga.show();
}