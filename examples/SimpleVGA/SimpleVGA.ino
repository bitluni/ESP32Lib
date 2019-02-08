#include <ESP32Lib.h>
#include <math.h>
#include <Ressources/Font6x8.h>

//pin configuration
const int redPin = 14;
const int greenPin = 19;
const int bluePin = 27;
const int hsyncPin = 0;
const int vsyncPin = 5;

//VGA Device
VGA3Bit vga;
//VGA14Bit vga;

void setup()
{
  Serial.begin(115200);
  //initializing i2s vga and frame buffers
  vga.init(vga.MODE320x240, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
  //vga.init(vga.MODE200x150, redPins, greenPins, bluePins, hsyncPin, vsyncPin);
  //select font
  vga.setFont(Font6x8);
}

void draw()
{
  static int lastMillis = 0;
  int t = millis();
  int fps = 1000 / (t - lastMillis);
  lastMillis = t;
  vga.clear();
  vga.setTextColor(0xffff);
  vga.setCursor(0, 0);
  vga.print("mem: ");
  vga.print((int)heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
  vga.print(" fps: ");
  vga.print(fps);
  vga.show();
}

void loop()
{
  draw();
}