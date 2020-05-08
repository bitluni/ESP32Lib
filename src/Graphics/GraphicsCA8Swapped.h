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

class GraphicsCA8Swapped: public Graphics<unsigned char>
{
	public:
	typedef unsigned char Color;

	int levelHighClipping = 255;
	int levelWhite = 207;
	int amplitudeBurst = 31;
	int levelBlack = 62;
	int levelBlanking = 62;
	int levelLowClipping = 14;
	int levelSync = 0;

	int firstPixelOffset = 0; // falling edge of hSync
	double colorClockPeriod = 1; // in pixels
	int bufferVDiv = 1;
	bool bufferInterlaced = false;
	bool bufferPhaseAlternating = false;

	GraphicsCA8Swapped()
	{
		frontColor = 0xff;
	}

	virtual int R(Color c) const
	{
		return (((int)c & 3) * 255 + 1) / 3;
	}
	virtual int G(Color c) const
	{
		return (((int)(c >> 2) & 3) * 255 + 1) / 3;
	}
	virtual int B(Color c) const
	{
		return (((int)(c >> 4) & 3) * 255 + 1) / 3;
	}
	virtual int A(Color c) const
	{
		return (((int)(c >> 6) & 3) * 255 + 1) / 3;
	}

	virtual Color RGBA(int r, int g, int b, int a = 255) const
	{
		return ((r >> 6) & 0b11) | ((g >> 4) & 0b1100) | ((b >> 2) & 0b110000) | (a & 0b11000000);
	}

	virtual void dotFast(int x, int y, Color color)
	{
		int r = R(color);
		int g = G(color);
		int b = B(color);
		
		float Y = 0.299*r + 0.587*g + 0.114*b; // range 0,255
		float B_Y = b - Y;// range +/- 225.93
		float R_Y = r - Y;// range +/- 178.755
		double phase = ((double)(x + firstPixelOffset)/colorClockPeriod)*(2*PI);
		float signal = 0;
		if(bufferPhaseAlternating)
		{
			//signal = Y + 0.492111*B_Y*sin(phase) + ((y&1==0)?1:-1)*0.877283*R_Y*cos(phase);
			//PENDING: DEBUG SOME ERROR: I NEEDED TO SWAP R_Y AND B_Y AND TO ADD 1/3*PI TO PHASE
			//TO GET COLORS IN ORDER (THEY APPEARED BGR AFTER BURST PHASE ADJUSTMENT)
			//THIS WORKS ON CRT TVs BUT NOT IN LCD
			signal = Y + 0.492111*R_Y*sin(phase + PI*1/3) + ((y&1==0)?1:-1)*0.877283*B_Y*cos(phase + PI*1/3);
		} else {
			signal = 0.925*Y + 0.075*255 + 0.925*0.492111*B_Y*sin(phase) + 0.925*0.877283*R_Y*cos(phase);
		}
		signal = levelBlanking + signal * (levelWhite - levelBlanking + 1) / 256;
		if (signal > levelHighClipping) signal = levelHighClipping;
		if (signal < levelLowClipping) signal = levelLowClipping;
		backBuffer[y][x^2] = ((uint8_t)signal);
	}

	virtual void dot(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			dotFast(x, y, color);
	}

	virtual void dotAdd(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			dotFast(x, y, color);
	}
	
	virtual void dotMix(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			dotFast(x, y, color);
	}
	
	virtual Color get(int x, int y)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			return backBuffer[y][x^2];
		return 0;
	}

	virtual void clear(Color color = 0)
	{
		for (int y = 0; y < this->yres; y++)
			for (int x = 0; x < this->xres; x++)
				dotFast(x, y, color);
	}

	virtual Color** allocateFrameBuffer()
	{
		return Graphics<Color>::allocateFrameBuffer(xres, yres, 0);
	}
};
