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
#include "Graphics.h"

class GraphicsPAL8Swapped: public Graphics<unsigned char>
{
	public:
	typedef unsigned char Color;
	static const Color BlackLevel = 76;
	short *SIN;
	short *COS;
	short YLUT[16];
	short UVLUT[16];
	float burstPerSample;
	
	GraphicsPAL8Swapped()
	{
		frontColor = 0xff;
	}

	void initLUTs(long pixelClock, long colorClock, int burstDelta, int hres)
	{
		burstPerSample = (2 * M_PI) / ((double)pixelClock / colorClock);
		SIN = (short*)malloc(hres * sizeof(short));
		COS = (short*)malloc(hres * sizeof(short));
		int maxLevel = 255;
		for(int i = 0; i < hres; i++)
		{
			int c = burstDelta + i;
			SIN[i] = round(0.436798f * sin(c * burstPerSample) * 256);
			COS[i] = round(0.614777f * cos(c * burstPerSample) * 256);     
		}
		for(int i = 0; i < 16; i++)
		{
			YLUT[i] = (int(BlackLevel) << 8) + round(i / 15.f * 256 * maxLevel);
			UVLUT[i] = round((i - 8) / 7.f * maxLevel);
		}
	}

	virtual int R(Color c) const
	{
		return ((int)(c - BlackLevel) * 255) / (255 - BlackLevel) ;
	}
	virtual int G(Color c) const
	{
		return ((int)(c - BlackLevel) * 255) / (255 - BlackLevel) ;
	}
	virtual int B(Color c) const
	{
		return ((int)(c - BlackLevel) * 255) / (255 - BlackLevel) ;
	}
	virtual int A(Color c) const
	{
		return 255;
	}

	virtual Color RGBA(int r, int g, int b, int a = 255) const
	{
		return (r * 2126 + g * 7152 + b * 722) * (255 - BlackLevel) / 2550000 + BlackLevel;
	}

	virtual void dotFast(int x, int y, Color color)
	{
		x <<= 1;
		backBuffer[y][x^2] = color;
		backBuffer[y][(x + 1)^2] = color;
		//backBuffer[y][(x + 2)^2] = color;
		//backBuffer[y][(x + 3)^2] = color;
	}

	virtual void dot(int x, int y, Color color)
	{
		x <<= 1;
		if ((unsigned int)x < xres && (unsigned int)y < yres)
		{
			backBuffer[y][x^2] = color;
			backBuffer[y][(x + 1)^2] = color;
			//backBuffer[y][(x + 2)^2] = color;
			//backBuffer[y][(x + 3)^2] = color;
		}
	}

	virtual void dotAdd(int x, int y, Color color)
	{
		dot(x, y, color);
/*		if ((unsigned int)x < xres && (unsigned int)y < yres)
		{
			int c0 = backBuffer[y][x^2];
			int c1 = color;
			int cn = c0 + c1 - BlackLevel;
			if(cn > 255) cn = 255;
			backBuffer[y][x^2] = cn;
		}*/
	}
	
	virtual void dotMix(int x, int y, Color color)
	{
		dot(x, y, color);
		/*if ((unsigned int)x < xres && (unsigned int)y < yres)
			backBuffer[y][x^2] = ((int)backBuffer[y][x^2] + color) / 2;*/
	}
	
	virtual Color get(int x, int y)
	{
		/*if ((unsigned int)x < xres && (unsigned int)y < yres)
			return backBuffer[y][x^2];*/
		return 0;
	}

	virtual void clear(Color color = 0)
	{
		for (int y = 0; y < this->yres; y++)
			for (int x = 0; x < this->xres; x++)
				backBuffer[y][x^2] = BlackLevel;
	}
	
	virtual Color** allocateFrameBuffer()
	{
		return Graphics<Color>::allocateFrameBuffer(xres, yres, (Color)BlackLevel);
	}
};