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
#include "ImageDrawer.h"
#include "Image.h"

class Sprite : public Image
{
  public:
	typedef Image::PixelFormat PixelFormat;
	unsigned char pointCount;
	const signed short (*points)[2];

	void init(int xres, int yres, const void *pixels, unsigned char pointCount, const signed short points[][2], PixelFormat pixelFormat)
	{
		static const signed short zeroPoint[][2] = {0, 0};
		Image::init(xres, yres, pixels, pixelFormat);
		if (pointCount)
		{
			this->pointCount = pointCount;
			this->points = points;
		}
		else
		{
			this->pointCount = 1;
			this->points = zeroPoint;
		}
	}
	void draw(ImageDrawer &g, int x, int y)
	{
		g.image(*this, x - points[0][0], y - points[0][1]);
	}

	void drawMix(ImageDrawer &g, int x, int y)
	{
		g.imageMix(*this, x - points[0][0], y - points[0][1]);
	}

	void drawAdd(ImageDrawer &g, int x, int y)
	{
		g.imageAdd(*this, x - points[0][0], y - points[0][1]);
	}
};

class Sprites
{
  public:
	typedef Image::PixelFormat PixelFormat;
	int count;
	Sprite *sprites;

	Sprites(int count, const void* pixels, const int offsets[], const unsigned short resolutions[][2], const signed short points[][2], const short pointOffsets[], PixelFormat pixelFormat)
	{
		this->count = count;
		this->sprites = new Sprite[count];
		for (int i = 0; i < count; i++)
			this->sprites[i].init(resolutions[i][0], resolutions[i][1], (unsigned char*)pixels + offsets[i], pointOffsets[i + 1] - pointOffsets[i], &points[pointOffsets[i]], pixelFormat);
	}

	void draw(ImageDrawer &g, int sprite, int x, int y)
	{
		sprites[sprite].draw(g, x, y);
	}

	void drawMix(ImageDrawer &g, int sprite, int x, int y)
	{
		sprites[sprite].drawMix(g, x, y);
	}

	void drawAdd(ImageDrawer &g, int sprite, int x, int y)
	{
		sprites[sprite].drawAdd(g, x, y);
	}

	int xres(int sprite) const
	{
		return sprites[sprite].xres;
	}

	int yres(int sprite) const
	{
		return sprites[sprite].yres;
	}

	const short (*points(int sprite) const)[2]
	{
		return sprites[sprite].points;
	}

	const short *point(int sprite, int point) const
	{
		return sprites[sprite].points[point];
	}

	/*unsigned short get(int sprite, int x, int y) const
	{
		return sprites[sprite].get(x, y);
	}*/
};
