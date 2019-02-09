#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>

//pin configuration
const int redPin = 14;
const int greenPin = 19;
const int bluePin = 27;
const int hsyncPin = 0;
const int vsyncPin = 5;

//VGA Device
VGA3Bit vga;

void setup()
{
	//initializing i2s vga and frame buffers
	vga.setFrameBufferCount(2);
	vga.init(vga.MODE320x200, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
	vga.setFont(Font6x8);
}

void loop()
{
	vga.clear(vga.RGB(0xffffff));
	vga.setCursor(10, 10);
	vga.setTextColor(8); //vga.RGB(0));
	vga.print("free memory: ");
	vga.print((int)heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
	vga.print(" fps: ");
	static int f = 0;
	vga.print(long((f++ * 1000) / millis()));
	vga.show();
}