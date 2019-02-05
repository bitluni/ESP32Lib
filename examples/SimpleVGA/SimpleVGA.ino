#include <ESP32Lib.h>

#include "gfx/font6x8.h"
Font<Graphics<Color>> font(6, 8, font6x8::pixels);

//Note that the I/O GPIO pads are 0-19, 21-23, 25-27, 32-39, while the output GPIOs are 0-19, 21-23, 25-27, 32-33. GPIO pads 34-39 are input-only
//0, 2, 4, 5, 12,  13, 14, 15, 16,  17, 18, 19, 21,  22, 23, 27, 32, 33
//0 (boot mode: use only as blank output)
//5 (LED)
//21 (SDA), 22(SCL)
//25, 26 (DAC)
//1(TX), 3(RX)

const int redPins[] = {-1, -1, -1, -1, 14};
const int greenPins[] = {-1, -1, -1, -1, 19};
const int bluePins[] = {-1, -1, -1, 27};
const int hsyncPin = 0;
const int vsyncPin = 5;

//VGA Device
I2SVGA graphics(1);

void setup()
{
  Serial.begin(115200);
  //initializing i2s vga and frame buffers
  graphics.init(graphics.MODE320x120, redPins, greenPins, bluePins, hsyncPin, vsyncPin, 16);
  //select font
  graphics.setFont(font);
}

void draw()
{
  static int lastMillis = 0;
  int t = millis();
  int fps = 1000 / (t - lastMillis);
  lastMillis = t;
  graphics.begin();
  graphics.clear();
  graphics.setTextColor(0xffff);
  graphics.setCursor(0, 0);
  graphics.print("mem: ");
  graphics.print((int)heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
  graphics.print(" fps: ");
  graphics.print(fps, 10, 2);
  graphics.end();
}

void loop()
{
  draw();
}
