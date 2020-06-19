#pragma once
#include <math.h>
long sinTab[256];

void calculateSinTab()
{
  for(int i = 0; i < 256; i++)
  {
    sinTab[i] = int(sin(M_PI / 128.f * i) * 256);
  }
}

float smoothstep(float x)
{
  x = max(0.f, min(x, 1.f));
  return (3 * x * x) - (2 * x * x * x);
}

float rsqrt(float number)
{
  long i;
  float x2, y;
  const float threehalfs = 1.5F;

  x2 = number * 0.5F;
  y  = number;
  i  = * ( long * ) &y;                       // evil floating point bit level hacking
  i  = 0x5f3759df - ( i >> 1 );               // what the fuck? 
  y  = * ( float * ) &i;
  y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
  y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed
  return y;
}

int rainbow[256][3];
void calcRainbow()
{
  const float cb[][3] = {{1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1}, {0, 0, 1}, {1, 0, 1}};
  for (int i = 0; i < 256; i++)
  {
    //interpolate the colors from the cb array and calculate the R5G5B5 color
    float s = 6.f / 256 * i;
    int n = int(s);
    float f = s - n;
    float fi = (1 - f);
    const float *cf = cb[n];
    const float *cfn = cb[(n + 1) % 6];
    int r = int((fi * cf[0] * 255) + (f * cfn[0] * 255));
    int g = int((fi * cf[1] * 255) + (f * cfn[1] * 255));
    int b = int((fi * cf[2] * 255) + (f * cfn[2] * 255));
    rainbow[i][0] = r;
    rainbow[i][1] = g;
    rainbow[i][2] = b;
  }
}

float rdist(int x, int y, float x2, float y2)
{
  return rsqrt((x - x2) * (x - x2) + (y - y2) * (y - y2) + 1);
}

float dist(int x, int y, float x2, float y2)
{
  return sqrt((x - x2) * (x - x2) + (y - y2) * (y - y2));
}

void getPixel(int x, int y, unsigned char *image, unsigned char *pixel)
{
  int p = image[(y * 160 + x) * 2 + 1] | (image[(y * 160 + x) * 2] << 8);
  pixel[0] = (p >> 11) << 3;
  pixel[1] = ((p >> 5) & 0b111111) << 2;
  pixel[2] = (p & 0b11111) << 3;
}

bool pixelMap(int x, int y, int &channel, int &led)
{
  if(x < 0 || x >= 48 || y < 0 || y >= 40) 
    return false;
  channel = x >> 3;
  led = (x & 7) + y * 8;
  return true;
}
