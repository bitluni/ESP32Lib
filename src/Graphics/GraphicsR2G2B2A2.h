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

class GraphicsR2G2B2A2: public Graphics<ColorR2G2B2A2, unsigned char>
{
	public:
	typedef unsigned char InternalColor;

	GraphicsR2G2B2A2()
	{
		frontColor = 0xff;
	}

	virtual void dotFast(int x, int y, Color color)
	{
		backBuffer[y][x] = color;
	}

	virtual Color getFast(int x, int y)
	{
		return backBuffer[y][x];
	}

	virtual InternalColor** allocateFrameBuffer()
	{
		return Graphics::allocateFrameBuffer(xres, yres, (InternalColor)0);
	}

	//re-evaluation of this method pending
	virtual void imageR2G2B2A2(Image &image, int x, int y, int srcX, int srcY, int srcXres, int srcYres)
	{
		for (int py = 0; py < srcYres; py++)
		{
			int i = srcX + (py + srcY) * image.xres;
			for (int px = 0; px < srcXres; px++)
				dot(px + x, py + y, ((unsigned char*)image.pixels)[i++]);
		}		
	}

	//re-evaluation of this method pending
	virtual void imageAddR2G2B2A2(Image &image, int x, int y, int srcX, int srcY, int srcXres, int srcYres)
	{
		for (int py = 0; py < srcYres; py++)
		{
			int i = srcX + (py + srcY) * image.xres;
			for (int px = 0; px < srcXres; px++)
				dotAdd(px + x, py + y, ((unsigned char*)image.pixels)[i++]);
		}
	}

	//re-evaluation of this method pending
	virtual void imageMixR2G2B2A2(Image &image, int x, int y, int srcX, int srcY, int srcXres, int srcYres)
	{
		for (int py = 0; py < srcYres; py++)
		{
			int i = srcX + (py + srcY) * image.xres;
			for (int px = 0; px < srcXres; px++)
				dotMix(px + x, py + y, ((unsigned char*)image.pixels)[i++]);
		}
	}	
};
