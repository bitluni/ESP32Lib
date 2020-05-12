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

class GraphicsX8CA8Swapped: public Graphics<ColorR5G5B4A2, unsigned short>
{
	public:
	typedef unsigned short InternalColor;
	// FUTURE PLANS: OUTPUTCOLOR COULD BE TEMPLATED

	int levelHighClipping = 95;
	int levelWhite = 77;
	int amplitudeBurst = 11;
	int levelBlack = 23;
	int levelBlanking = 23;
	int levelLowClipping = 5;
	int levelSync = 0;

	int firstPixelOffset = 0; // falling edge of hSync
	double colorClockPeriod = 1; // in pixels
	int bufferVDiv = 1;
	bool bufferInterlaced = false;
	bool bufferPhaseAlternating = false;

	GraphicsX8CA8Swapped()
	{
		frontColor = 0xffff;
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
		backBuffer[y][x^1] = ((uint8_t)signal) << 8;
	}

	virtual Color getFast(int x, int y)
	{
		return 0; // Not possible to retrieve color from this buffer
	}

	virtual InternalColor** allocateFrameBuffer()
	{
		return Graphics::allocateFrameBuffer(xres, yres, (InternalColor)(levelBlack << 8));
	}

	virtual void dotAdd(int x, int y, Color color)
	{
		dot(x, y, color);
	}
	
	virtual void dotMix(int x, int y, Color color)
	{
		dot(x, y, color);
	}
};
