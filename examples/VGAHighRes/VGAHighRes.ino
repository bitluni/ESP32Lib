#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>

//pin configuration
const int redPin = 14;
const int greenPin = 19;
const int bluePin = 27;
const int hsyncPin = 0;
const int vsyncPin = 5;

//VGA Device using an interrupt to unpack the pixels from 4bit to 16bit for the I²S
//This takes some CPU time in the background but is able to fit two frame buffers in the memory
VGA3BitI vga;

void setup()
{
	//enabling double buffering
	vga.setFrameBufferCount(2);
	//initializing the graphics mode
	vga.init(vga.MODE460x480, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
	//setting the font
	vga.setFont(Font6x8);
}

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

///draws a bounsing ball
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
	const int r = 80;
	int rx = r;
	int ry = r;
	vy += -9.81f * dt * 100;
	x += vx;
	y += vy * dt;
	//check for boundaries and bounce back
	if (y < r && vy < 0)
	{
		vy = 800;
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
	//clearing with white background
	vga.clear(vga.RGB(0xffffff));
	//text position
	vga.setCursor(10, 10);
	//black text color no background color
	vga.setTextColor(vga.RGB(0));
	//show the remaining memory
	vga.print("free memory: ");
	vga.print((int)heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
	//draw bouncing ball
	ball();
	//draw the logo
	bitluni(170, 160, 5);
	//show the backbuffer (only needed when using backbuffering)
	vga.show();
}