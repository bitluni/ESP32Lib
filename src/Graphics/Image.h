#pragma once

template<class Graphics>
class Image
{
  public:
  int xres;
  int yres;
  const unsigned char *pixels;
  
  Image(int xres_, int yres_, const unsigned char *pixels_)
    :xres(xres_),
    yres(yres_),
    pixels(pixels_)
  {
  }

  ~Image()
  {
  }

  void draw(Graphics &g, int x, int y)
  {
    int i = 0;
    for(int py = 0; py < yres; py++)
      for(int px = 0; px < xres; px++)
        g.dot(px + x, py + y, pixels[i++]);
  }

  void draw(Graphics &g, int x, int y, int srcX, int srcY, int srcXres, int srcYres)
  {
    for(int py = 0; py < srcYres; py++)
    {
      int i = srcX + (py + srcY) * xres;
      for(int px = 0; px < srcXres; px++)
        g.dot(px + x, py + y, pixels[i++]);
    }
  }
  
  void draw(Graphics &g, int x, int y, int t)
  {
    int i = 0;
    for(int py = 0; py < yres; py++)
      for(int px = 0; px < xres; px++)
      {
        int c = pixels[i++];
        if(c != t) g.dot(px + x, py + y, c);
      }
  }
  
  void drawAdd(Graphics &g, int x, int y)
  {
    int i = 0;
    for(int py = 0; py < yres; py++)
      for(int px = 0; px < xres; px++)
        g.dotAdd(px + x, py + y, pixels[i++]);
  }
};

