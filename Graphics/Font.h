#pragma once

template<class Graphics>
class Font
{
  public:
  int xres;
  int yres;
  const unsigned char *pixels;
  
  Font(int charWidth, int charHeight, const unsigned char *pixels_)
    :xres(charWidth),
    yres(charHeight),
    pixels(pixels_)
  {
  }

  void drawChar(Graphics &g, int x, int y, char ch, int frontColor, int backColor)
  {
    const unsigned char *pix = &pixels[xres * yres * (ch - 32)];
    for(int py = 0; py < yres; py++)
      for(int px = 0; px < xres; px++)
        if(*(pix++))
          g.dot(px + x, py + y, frontColor);
        else
        if(backColor >= 0)
          g.dot(px + x, py + y, backColor);
  }
};

