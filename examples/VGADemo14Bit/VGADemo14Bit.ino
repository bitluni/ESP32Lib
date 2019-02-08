#include <ESP32Lib.h>
#include <math.h>

//pin configuration
const int redPins[] = {2, 4, 12, 13, 14};
const int greenPins[] = {15, 16, 17, 18, 19};
const int bluePins[] = {21, 22, 23, 27};
const int hsyncPin = 0;
const int vsyncPin = 5;

//VGA Device
VGA14Bit vga;

void setup()
{
	//initializing i2s vga and frame buffers
	vga.init(vga.MODE200x150, redPins, greenPins, bluePins, hsyncPin, vsyncPin);
}

void loop()
{
	float factors[][2] = {{1, 1.1f}, {0.9f, 1.02f}, {1.1, 0.8}};
	int colors[] = {vga.RGB(0xff0000), vga.RGB(0x00ff00), vga.RGB(0x0000ff)};
	float p = millis() * 0.002f;
	for (int i = 0; i < 3; i++)
	{
		int x = vga.xres / 2 + sin(p * factors[i][0]) * vga.xres / 3;
		int y = vga.yres / 2 + cos(p * factors[i][1]) * vga.yres / 3;
		vga.fillCircle(x, y, 8, 0);
		vga.circle(x, y, 10, colors[i]);
	}
	for (int y = 0; y < vga.yres - 1; y++)
		for (int x = 1; x < vga.xres - 1; x++)
		{
			int c0 = vga.get(x, y) & 0x3fff;
			int c1 = vga.get(x, y + 1) & 0x3fff;
			int c2 = vga.get(x - 1, y + 1) & 0x3fff;
			int c3 = vga.get(x + 1, y + 1) & 0x3fff;
			int r = ((c0 & 0x1f) + (c1 & 0x1f) + ((c2 & 0x1f) + (c3 & 0x1f)) / 2) / 3;
			int g = (((c0 & 0x3e0) + (c1 & 0x3e0) + ((c2 & 0x3e0) + (c3 & 0x3e0)) / 2) / 3) & 0x3e0;
			int b = (((c0 & 0x3c00) + (c1 & 0x3c00) + ((c2 & 0x3c00) + (c3 & 0x3c00)) / 2) / 3) & 0x3c00;
			vga.dotFast(x, y, r | g | b);
		}
}
