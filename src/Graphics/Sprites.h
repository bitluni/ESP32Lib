#pragma once
#include "Image.h"

template <typename Color>
class Sprite : public Image<Color>
{
  public:
	unsigned char pointCount;
	const signed short (*points)[2];

	void init(int xres, int yres, const Color *pixels, unsigned char pointCount, const signed short points[][2])
	{
		static const signed short zeroPoint[][2] = {0, 0};
		Image<Color>::init(xres, yres, pixels);
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

	void draw(Graphics<Color> &g, int x, int y)
	{
		Image<Color>::draw(g, x - points[0][0], y - points[0][1]);
	}

	void drawMix(Graphics<Color> &g, int x, int y)
	{
		Image<Color>::drawMix(g, x - points[0][0], y - points[0][1]);
	}

	void drawAdd(Graphics<Color> &g, int x, int y)
	{
		Image<Color>::drawAdd(g, x - points[0][0], y - points[0][1]);
	}
};

template<typename Color>
class Sprites
{
  public:
	int count;
	Sprite<Color> *sprites;

	Sprites(int count, const Color pixels[], const int offsets[], const unsigned short resolutions[][2], const signed short points[][2], const short pointOffsets[])
	{
		this->count = count;
		this->sprites = new Sprite<Color>[count];
		for (int i = 0; i < count; i++)
			this->sprites[i].init(resolutions[i][0], resolutions[i][1], &pixels[offsets[i]], pointOffsets[i + 1] - pointOffsets[i], &points[pointOffsets[i]]);
	}

	void draw(Graphics<Color> &g, int sprite, int x, int y)
	{
		sprites[sprite].draw(g, x, y);
	}

	void drawMix(Graphics<Color> &g, int sprite, int x, int y)
	{
		sprites[sprite].drawMix(g, x, y);
	}

	void drawAdd(Graphics<Color> &g, int sprite, int x, int y)
	{
		sprites[sprite].drawMix(g, x, y);
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
