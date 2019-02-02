/*
	Author: bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 2.0
	https://creativecommons.org/licenses/by-sa/2.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
		http://bitluni.net
*/
#pragma once

template <class Graphics>
class Font
{
  public:
	int xres;
	int yres;
	const unsigned char *pixels;

	Font(int charWidth, int charHeight, const unsigned char *pixels_)
		: xres(charWidth),
		  yres(charHeight),
		  pixels(pixels_)
	{
	}

	void drawChar(Graphics &g, int x, int y, char ch, int frontColor, int backColor)
	{
		const unsigned char *pix = &pixels[xres * yres * (ch - 32)];
		for (int py = 0; py < yres; py++)
			for (int px = 0; px < xres; px++)
				if (*(pix++))
					g.dot(px + x, py + y, frontColor);
				else if (backColor >= 0)
					g.dot(px + x, py + y, backColor);
	}
};
