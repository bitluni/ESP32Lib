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
#include "Graphics.h"
#include "Image.h"

template <class Graphics>
class Sprite : public Image<Graphics>
{
  public:
	typedef typename Graphics::Color Color;
	unsigned char pointCount;
	const signed short (*points)[2];

	void init(int xres, int yres, const Color *pixels, unsigned char pointCount, const signed short points[][2])
	{
		static const signed short zeroPoint[][2] = {0, 0};
		Image<Graphics>::init(xres, yres, pixels);
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
	void draw(Graphics &g, int x, int y)
	{
		Image<Graphics>::draw(g, x - points[0][0], y - points[0][1]);
	}

	void drawMix(Graphics &g, int x, int y)
	{
		Image<Graphics>::drawMix(g, x - points[0][0], y - points[0][1]);
	}

	void drawAdd(Graphics &g, int x, int y)
	{
		Image<Graphics>::drawAdd(g, x - points[0][0], y - points[0][1]);
	}
};

template <class Graphics>
class Sprites
{
  public:
	typedef typename Graphics::Color Color;
	int count;
	Sprite<Graphics> *sprites;

	Sprites(int count, const Color pixels[], const int offsets[], const unsigned short resolutions[][2], const signed short points[][2], const short pointOffsets[])
	{
		this->count = count;
		this->sprites = new Sprite<Graphics>[count];
		for (int i = 0; i < count; i++)
			this->sprites[i].init(resolutions[i][0], resolutions[i][1], &pixels[offsets[i]], pointOffsets[i + 1] - pointOffsets[i], &points[pointOffsets[i]]);
	}

	void draw(Graphics &g, int sprite, int x, int y)
	{
		sprites[sprite].draw(g, x, y);
	}

	void drawMix(Graphics &g, int sprite, int x, int y)
	{
		sprites[sprite].drawMix(g, x, y);
	}

	void drawAdd(Graphics &g, int sprite, int x, int y)
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

	unsigned short get(int sprite, int x, int y) const
	{
		return sprites[sprite].get(x, y);
	}
};
