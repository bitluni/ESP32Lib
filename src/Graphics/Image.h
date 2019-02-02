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

template <class Color>
class Image
{
  public:
	int xres;
	int yres;
	const Color *pixels;

	void init(int xres, int yres, const unsigned short *pixels)
	{
		this->xres = xres;
		this->yres = yres;
		this->pixels = pixels;    
	}

	Image(){};

	Image(int xres, int yres, const unsigned short *pixels)
	{
		init(xres, yres, pixels);
	}
	
	~Image()
	{
	}

	void draw(Graphics<Color> &g, int x, int y)
	{
		int i = 0;
		for (int py = 0; py < yres; py++)
			for (int px = 0; px < xres; px++)
				g.dot(px + x, py + y, pixels[i++]);
	}

	void draw(Graphics<Color> &g, int x, int y, int srcX, int srcY, int srcXres, int srcYres)
	{
		for (int py = 0; py < srcYres; py++)
		{
			int i = srcX + (py + srcY) * xres;
			for (int px = 0; px < srcXres; px++)
				g.dot(px + x, py + y, pixels[i++]);
		}
	}

	void draw(Graphics<Color> &g, int x, int y, int t)
	{
		int i = 0;
		for (int py = 0; py < yres; py++)
			for (int px = 0; px < xres; px++)
			{
				int c = pixels[i++];
				if (c != t)
					g.dot(px + x, py + y, c);
			}
	}

	void drawAdd(Graphics<Color> &g, int x, int y)
	{
		int i = 0;
		for (int py = 0; py < yres; py++)
			for (int px = 0; px < xres; px++)
				g.dotAdd(px + x, py + y, pixels[i++]);
	}
	
	void drawMix(Graphics<Color> &g, int x, int y)
	{
		int i = 0;
		for(int py = 0; py < yres; py++)
			for(int px = 0; px < xres; px++)
				g.dotMix(px + x, py + y, pixels[i++]);
	}	
  
	Color get(int x, int y) const
	{
		return pixels[y * xres + x];
	}
};
