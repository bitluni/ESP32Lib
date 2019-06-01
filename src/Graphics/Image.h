/*
	Author: bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
		http://bitluni.net
*/
#pragma once

class Image
{
  public:
	enum PixelFormat
	{
		R8G8B8A8 = 1,
		R1G1B1A1 = 2,
		R2G2B2A2 = 3,
		R4G4B4A4 = 4,
		R5G5B5A1 = 5,
		R5G5B4A2 = 6
	};

	int xres;
	int yres;
	PixelFormat pixelFormat;
	const void *pixels;

	void init(int xres, int yres, const void *pixels, PixelFormat pixelFormat)
	{
		this->xres = xres;
		this->yres = yres;
		this->pixels = pixels;
		this->pixelFormat = pixelFormat;
	}

	Image(){};

	Image(int xres, int yres, const void *pixels, PixelFormat pixelFormat)
	{
		init(xres, yres, pixels, pixelFormat);
	}

	/*
	Color get(int x, int y) const
	{
		return pixels[y * xres + x];
	}*/
};
