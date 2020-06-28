/*
	Author: Martin-Laclaustra 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once
#include "Graphics.h"

class GraphicsCA8Swapped: public Graphics<ColorR5G5B4A2, unsigned char>
{
	public:
	typedef unsigned char InternalColor;
	// FUTURE PLANS: OUTPUTCOLOR COULD BE TEMPLATED

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
		frontColor = 0xffff;
	}

	virtual void dotFast(int x, int y, Color color)
	{
		
		// ranges 0,255
		int r = R(color);
		int g = G(color);
		int b = B(color);
		
		float Y = 0.299*r + 0.587*g + 0.114*b; // range 0,255
		float B_Y = b - Y;// range +/- 225.93
		float R_Y = r - Y;// range +/- 178.755
		
		// u in YUV
		float b_y = 0.492111*(B_Y); // range +/- 111.18
		// v in YUV
		float r_y = 0.877283*(R_Y); // range +/- 156.82
		
		// float sat = sqrt(pow(b_y,2) + pow(r_y,2)); // range 0,192
		// float hue_phase = atan2(r_y,b_y); // range -PI,PI
		
		// position_phase = fmod(x + firstPixelOffset,colorClockPeriod)*2*PI/colorClockPeriod; // range 0,2*PI
		double position_phase = (double)(x + firstPixelOffset)*2*PI/colorClockPeriod;
		
		float signal = 0; // nominally the Y range, 0,255, but color excursion avobe and bellow are possible
		
		if(bufferPhaseAlternating)
		{
			// signal = Y + sat*sin((((int)(y & 1)==0)?((float)(-1)):((float)(1)))*hue_phase + position_phase);
			signal = Y + b_y*sin(position_phase) + (((int)(y & 1)==0)?((float)(-1)):((float)(1)))*r_y*cos(position_phase);
		} else {
			// signal = 0.925*Y + 0.075*255 + 0.925*sat*sin(hue_phase + position_phase);
			signal = 0.925*Y + 0.075*255 + 0.925*(b_y*sin(position_phase) + r_y*cos(position_phase));
		}
		signal = levelBlanking + signal * (levelWhite - levelBlanking + 1) / 256;
		if (signal > levelHighClipping) signal = levelHighClipping;
		if (signal < levelLowClipping) signal = levelLowClipping;
		backBuffer[y][x^2] = ((uint8_t)signal);
	}

	virtual Color getFast(int x, int y)
	{
		return 0; // Not possible to retrieve color from this buffer
	}

	virtual InternalColor** allocateFrameBuffer()
	{
		return Graphics::allocateFrameBuffer(xres, yres, (InternalColor)levelBlack);
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
