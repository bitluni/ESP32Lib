//A realtime raytraycer utilizing both cores of the ESP32. Please change the pin configuration in the setup if you are not using VGA v0.1 or the Black Edition shields.
//cc by-sa 4.0 license
//bitluni

#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>
#include <math.h>
#include <Math/Matrix.h>
#include "Raytracer.h"

//VGA Device
VGA14Bit vga;

int taskData[2][3] = 
  {
    {0, 0, 160},
    {0, 160, 320} 
  };

static Sphere sphere(Vector(0, 0.5f, 0), 1);
static Sphere sphere2(Vector(1, 1.5f, 0.5), 0.5f);
static Checker checker;
static Raytracable *objects[] = {&sphere, &sphere2, &checker};
static Vector light = Vector(5, 4, -5);

void raytraceTask(void *param)
{
  static Vector p(0, 1, -10);
  int *data = (int*)param;
  while(true)
  {
    while(!data[0]) delay(1);
    for(int y = 0; y < 200; y++)
      for(int x = data[1]; x < data[2]; x++)
      {
        Vector v(float(x - 160) * (1.f / 320), float(100 - y) * (1.f / 320), 1.f);
        v.normalize();
        Ray r(p, v);
        Vector c = raytrace(objects, 3, r, light, 3);
        vga.dotFast(x, y, vga.RGB(c[0] * 255, c[1] * 255, c[2] * 255));
      }
    data[0] = 0;
  }
}

//initial setup
void setup()
{
  //we need double buffering for smooth animations
  vga.setFrameBufferCount(2);  
	//initializing i2s vga
  //Pin presets are avaialable for: VGAv01, VGABlackEdition, VGAWhiteEdition, PicoVGA
  //But you can also use custom pins. Check the other examples  
	vga.init(vga.MODE320x200, vga.VGABlackEdition);
	//setting the font
	vga.setFont(Font6x8);
  light.normalize();
  sphere.reflection = 0.4f;
  sphere2.reflection = 0.5f;
  checker.reflection = 0.2f;
  sphere.c = Vector(0, 1, 0);
  sphere2.c = Vector(1, 0, 1);
  static uint8_t ucParameterToPass;
  TaskHandle_t xHandle = NULL;
  xTaskCreatePinnedToCore(raytraceTask, "Raytracer1", 2000, taskData[0],  ( 2 | portPRIVILEGE_BIT ), &xHandle, 0);
  xTaskCreatePinnedToCore(raytraceTask, "Raytracer2", 2000, taskData[1],  ( 2 | portPRIVILEGE_BIT ), &xHandle, 1);
}

//the loop is done every frame
void loop()
{  
  taskData[0][0] = 1;
  taskData[1][0] = 1;
  //waiting for task to finish
  while(taskData[0][0] || taskData[1][0]) delay(1);
  sphere2.p.v[0] = sin(millis() * 0.0005f) * 2;
  sphere2.p.v[2] = cos(millis() * 0.0005f) * 2;
  sphere.p.v[1] = sphere2.p[0] * 0.3 + 1;
  //setting the text cursor to the lower left corner of the screen
  vga.setCursor(0, 0);
  //setting the text color to white with opaque black background
  vga.setTextColor(vga.RGB(0xffffff), vga.RGBA(0, 0, 0, 0));
  //printing the fps
  vga.print("ms/frame: ");
  static long t = 0;
  long ct = millis();
  vga.print(ct - t);
  t = ct;
  vga.show();
}
