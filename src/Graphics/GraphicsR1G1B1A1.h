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
#include "TriangleTree.h"

class GraphicsR1G1B1A1: public Graphics<unsigned char>
{
	public:
	typedef unsigned char Color;
	Color **frame;
	Color **backbuffer;

	virtual void dotFast(int x, int y, Color color)
	{
		if(x & 1)
			backbuffer[y][x >> 1] = backbuffer[y][x >> 1] | (color << 4);
		else
			backbuffer[y][x >> 1] = backbuffer[y][x >> 1] | color;
	}

	virtual void dot(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			if(x & 1)
				backbuffer[y][x >> 1] = (backbuffer[y][x >> 1] & 0xf) | (color << 4);
			else
				backbuffer[y][x >> 1] = (backbuffer[y][x >> 1] & 0xf0) | (color & 0xf);
	}

	virtual void dotAdd(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			if(x & 1)
				backbuffer[y][x >> 1] = backbuffer[y][x >> 1] | (color << 4);
			else
				backbuffer[y][x >> 1] = backbuffer[y][x >> 1] | (color & 0xf);
	}
	
	virtual void dotMix(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres && (color & 8) != 0)
		{
			if(x & 1)
				backbuffer[y][x >> 1] = backbuffer[y][x >> 1] | (color << 4);
			else
				backbuffer[y][x >> 1] = backbuffer[y][x >> 1] | (color & 0xf);
		}	
	}
	
	virtual char get(int x, int y)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			if(x & 1)
				return backbuffer[y][x >> 1] = backbuffer[y][x >> 1] >> 4;
			else
				return backbuffer[y][x >> 1] = backbuffer[y][x >> 1] & 0xf;
		return 0;
	}

	virtual void clear(Color clear = 0)
	{
		for (int y = 0; y < this->yres; y++)
			for (int x = 0; x < this->xres / 2; x++)
				this->backbuffer[y][x] = clear | (clear << 4);
	}

	virtual void show()
	{
		Color **b = backbuffer;
		backbuffer = frame;
		frame = b;
	}

	virtual void initBuffers(const bool doubleBuffer = true, const bool zbuffer = false)
	{
		frame = (Color **)malloc(yres * sizeof(Color *));
		if(doubleBuffer)
			backbuffer = (Color **)malloc(yres * sizeof(Color *));
		else
			backbuffer = frame;
		//not enough memory for z-buffer implementation
		//zbuffer = (char**)malloc(yres * sizeof(char*));
		for (int y = 0; y < yres; y++)
		{
			frame[y] = (Color *)malloc(xres / 2 * sizeof(Color));
			if(doubleBuffer)
				backbuffer[y] = (Color *)malloc(xres / 2 * sizeof(Color));
			//zbuffer[y] = (char*)malloc(xres);
		}
	}
};