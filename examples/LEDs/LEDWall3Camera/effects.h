#pragma once

unsigned char newImage[40][48][3];

void downSample(unsigned char *frame)
{
  //160x120
  for(int y = 0; y < 40; y++)
  {
    for(int x = 0; x < 48; x++)
    {
      int r = 0;
      int g = 0;
      int b = 0;
      for(int j = 0; j < 3; j++)
        for(int i = 0; i < 3; i++)
        {
          unsigned char p[3];          
          getPixel(8 + x * 3 + i, y * 3 + j, frame, p);
          r += p[0];
          g += p[1];
          b += p[2];
        }
      newImage[y][x][0] = (r * (256 / 9)) >> 8;
      newImage[y][x][1] = (g * (256 / 9)) >> 8;
      newImage[y][x][2] = (b * (256 / 9)) >> 8;
    }
  }
}

void showImage()
{
  for(int y = 0; y < 40; y++)
    for(int x = 0; x < 48; x++)
    {
      int channel = 0, led = 0;
      pixelMap(x, y, channel, led);
      gfx.setLED(channel, led, newImage[y][x][1], newImage[y][x][0], newImage[y][x][2], 0);
    }
  gfx.show();
}


void processImage()
{
  showImage();
}