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

int heat(VGA14Bit::Color c)
{
	if (vga.B(c))
		return vga.B(c) + 512;
	if (vga.G(c))
		return vga.G(c) + 256;
	return vga.R(c);
}

VGA14Bit::Color color(int heat)
{
	if (heat >= 512)
		return vga.RGB(255, 255, heat - 512);
	if (heat >= 256)
		return vga.RGB(255, heat - 256, 0);
	return vga.RGB(heat, 0, 0);
}

void loop()
{
	float p = millis() * 0.001f;
	vga.fillCircle(vga.xres / 2 + sin(p) * vga.xres / 3, vga.yres / 2 + cos(p * 1.34f) * vga.yres / 3, (rand() & 1) + 3, vga.RGB(255, 255, 255));
	for (int y = 0; y < vga.yres - 1; y++)
		for (int x = 1; x < vga.xres - 1; x++)
		{
			int h = (heat(vga.get(x, y)) + (heat(vga.get(x - 1, y + 1)) + heat(vga.get(x + 1, y + 1))) / 2 + heat(vga.get(x, y + 1))) / 3;
			vga.dotFast(x, y, color(h));
		}
	for (int x = 0; x < vga.xres; x++)
		vga.dotFast(x, vga.yres - 1, (rand() & 3 == 3) ? color(767) : 0);
}