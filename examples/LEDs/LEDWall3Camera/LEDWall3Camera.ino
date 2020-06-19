//example for parallel LEDs not using the graphics interface
//using the library
#include <ESP32Lib.h>
#include "camera.h"
#include "tools.h"

ParallelLED gfx(4); //four color components for RGBW

volatile bool frameAvailable = false;
#include "effects.h"

void setup()
{
  Serial.begin(115200);
  cameraInit();
  setCameraParams();
  gfx.setGamma(3.2f, 2.0f, 4.0f, 3.2f); //gamma adjustment
  const int pins[] = {16, 12, 13, 15, 14, 2, -1, -1};
  gfx.init(pins, 8 * 40); //8 x 40 LEDs per channel (8 channels total, here only 6 used)
}

void loop()
{
  camera_fb_t * fb = esp_camera_fb_get();
  if (fb) 
  {
    downSample(fb->buf);  //fb->width, fb->height, fb->format, fb->buf, fb->len
    frameAvailable = true;
    esp_camera_fb_return(fb);
  };   
  if(frameAvailable) 
  {
    processImage();
    frameAvailable = false;
  }
}