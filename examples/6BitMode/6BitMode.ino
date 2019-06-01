//This example shows a rendering of Julia set. Please change the pinConfig if you are using a different board to PicoVGA
//cc by-sa 4.0 license
//bitluni

//include libraries
#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>

//VGA Device
VGA6Bit vga;
//Pin presets are avaialable for: VGAv01, VGABlackEdition, VGAWhiteEdition, PicoVGA
const PinConfig &pinConfig = VGA6Bit::PicoVGA;

int taskData[2][3] = 
  {
    {0, 0, 160},
    {0, 160, 320} 
  };

static float v = -1.5;
static float vs = 0.001;

//https://en.wikipedia.org/wiki/Julia_set#Pseudocode_for_normal_Julia_sets
int julia(int x, int y, float cx, float cy)
{
  int zx = ((x - 159.5f) * (1.f / 320.f * 5.0f)) * (1 << 12);
  int zy = ((y - 99.5f) * (1.f / 200.f * 3.0f)) * (1 << 12);
  int i = 0;
  const int maxi = 17;
  int cxi = cx ;
  int cyi = cy * (1 << 12);
  while(zx * zx + zy * zy < (4 << 24) && i < maxi) 
  {
    int xtemp = (zx * zx - zy * zy) >> 12;
    zy = ((zx * zy) >> 11) + cyi; 
    zx = xtemp + cxi;
    i++;
  }
  return i;
}

int colors[] = {
  0b110001, 0b110010, 0b110011, 0b100011, 0b010011,
  0b000011, 0b000111, 0b001011, 0b001111, 0b001110, 0b001101, 
  0b001100, 0b011100, 0b101100, 0b111100, 0b111000, 0b110100, 
  0b110000};

void renderTask(void *param)
{
  int *data = (int*)param;
  while(true)
  {
    while(!data[0]) delay(1);
    for(int y = 0; y < 100; y++)
      for(int x = data[1]; x < data[2]; x++)
      {
        int c = colors[julia(x, y, -0.74543f, v)];
        vga.dotFast(x, y, c);
        vga.dotFast(319 - x, 199 - y, c);
      }
    data[0] = 0;
  }
}

//initial setup
void setup()
{
	//initializing i2s vga (with only one framebuffer)
	vga.init(vga.MODE320x200, pinConfig);
  TaskHandle_t xHandle = NULL;
  xTaskCreatePinnedToCore(renderTask, "Render1", 2000, taskData[0],  ( 2 | portPRIVILEGE_BIT ), &xHandle, 0);
  xTaskCreatePinnedToCore(renderTask, "Render2", 2000, taskData[1],  ( 2 | portPRIVILEGE_BIT ), &xHandle, 1);
}
  
//just draw each frame
void loop()
{
  static unsigned long ot = 0;
  unsigned long t = millis();
  unsigned long dt =  t - ot;
  ot = t;
  taskData[0][0] = 1;
  taskData[1][0] = 1;
  //waiting for task to finish
  while(taskData[0][0] || taskData[1][0]) delay(1);
  v += vs * dt;
  if(v > 1.5f)
  {
    v = 1.5f;
    vs = -vs;
  }
  if(v < -1.5f)
  {
    v = -1.5f;
    vs = -vs;
  }
}
