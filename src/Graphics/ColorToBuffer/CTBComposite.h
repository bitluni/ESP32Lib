/*
	Author: Martin-Laclaustra 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

#include "../integertrigonometry.h"
#include "../Colors/InterfaceColors_ColorR8G8B8A8.h"

class CTBComposite
{
	public:
	CTBComposite() {}

	int levelHighClipping = 255;
	int levelWhite = 207;
	int amplitudeBurst = 31;
	int levelBlack = 62;
	int levelBlanking = 62;
	int levelLowClipping = 14;
	int levelSync = 0;

	//int levelHighClipping = 95;
	//int levelWhite = 77;
	//int amplitudeBurst = 11;
	//int levelBlack = 23;
	//int levelBlanking = 23;
	//int levelLowClipping = 5;
	//int levelSync = 0;

	int firstPixelOffset = 0; // falling edge of hSync
	uint32_t colorClock0x1000Periods = 1; // pixels per 0x1000 color cycles
	int bufferVDiv = 1;
	bool bufferInterlaced = false;
	bool bufferPhaseAlternating = false;

	int coltobuf(int val, int x, int y)
	{
		
		// ranges 0,255
		uint8_t r = ColorR8G8B8A8::static_R(val);
		uint8_t g = ColorR8G8B8A8::static_G(val);
		uint8_t b = ColorR8G8B8A8::static_B(val);
		
		// Y = 0.299*r + 0.587*g + 0.114*b; // range 0,255
		// Y = ( 0x00010000*0.299*r + 0x00010000*0.587*g + 0x00010000*0.114*b + 0x8000L) / 0x00010000
		uint8_t Y = (19595L*r + 38470L*g + 7471L*b + 0x8000L)>>16;
		// u in YUV
		// b_y = 0.492111*(b - Y); // range +/- 111.18
		// b_y = (0x00010000*0.492111*(b - Y) + 0x08008000 - 1) / 0x00010000 - 0x0800
		int32_t b_y = (int32_t)(((uint32_t)(32251L*((int32_t)b - Y) + 0x08007fffL))>>16) - 0x0800;
		// v in YUV
		// r_y = 0.877283*(r - Y); // range +/- 156.82
		// r_y = (0x00010000*0.877283*(b - Y) + 0x08008000 - 1) / 0x00010000 - 0x0800
		int32_t r_y = (int32_t)(((uint32_t)(57494L*((int32_t)r - Y) + 0x08007fffL))>>16) - 0x0800;
		
		// sat = sqrt(pow(b_y,2) + pow(r_y,2)); // range 0,192
		// alpha max plus beta min algorithm, using alpha = 15/16 and beta = 15/32
		uint8_t sat = (abs(b_y) > abs(r_y)) ? (15*((abs(b_y)<<1) + abs(r_y)))>>5 : (15*(abs(b_y) + (abs(r_y)<<1)))>>5;
		// hue_phase = atan2(r_y,b_y); // range -PI,PI
		// hue_phase = fmod(atan2(r_y,b_y)+2*PI,2*PI)*256/(2*PI); // range 0,255
		int32_t hue_phase = integeratan2aprox(r_y,b_y)>>8;
		
		// position_phase = fmod(x + firstPixelOffset,colorClockPeriod)*256/colorClockPeriod; // range 0,255 // colorClockPeriod in pixels (= pixelClock/colorClock)
		uint32_t position_phase = ((((x + firstPixelOffset)<<12)%colorClock0x1000Periods)<<8)/colorClock0x1000Periods;
		
		int32_t signal = 0; // nominally the Y range, 0,255, but color excursion avobe and bellow are possible
		
		if(bufferPhaseAlternating)
		{
			// signal = Y + b_y*sin(position_phase) + (((int)(y & 1)==0)?((float)(-1)):((float)(1)))*r_y*cos(position_phase);
			// signal = Y + sat*sin((((int)(y & 1)==0)?((float)(-1)):((float)(1)))*hue_phase + position_phase);
			// signal 0,1 mapped to 0, (255*(0x01L<<(7+7))) to preserve precision in subsequent re-scaling
			// signal = (Y + sat*sin((((int)(y & 1)==0)?((float)(-1)):((float)(1)))*hue_phase + position_phase))*(0x01L<<(7+7));
			// signal = (0x01L<<(7+7))*Y + (0x01L<<(7+7))*sat*sin((((int)(y & 1)==0)?((float)(-1)):((float)(1)))*hue_phase + position_phase);
			// sin ranges -1, 1 for arguments -PI,PI; integersinaprox ranges -127, 127 for arguments 0, 255
			signal = (0x01L<<(7+7))*Y + (0x01L<<(7))*sat*integersinaprox((((int)(y & 1)==0)?((int)(-1)):((int)(1)))*hue_phase + position_phase);
		} else {
			// signal = 0.925*Y + 0.075*255 + 0.925*(b_y*sin(position_phase) + r_y*cos(position_phase));
			// signal = 0.925*Y + 0.075*255 + 0.925*sat*sin(hue_phase + position_phase);
			// signal 0,1 mapped to 0, (255*(0x01L<<(7+7))) to preserve precision in subsequent re-scaling
			// signal = (0.925*Y + 0.075*255 + 0.925*sat*sin(hue_phase + position_phase))*(0x01L<<(7+7));
			// signal = 0.925*(0x01L<<(7+7))*Y + 0.075*255*(0x01L<<(7+7)) + 0.925*(0x01L<<(7+7))*sat*sin(hue_phase + position_phase);
			// sin ranges -1, 1 for arguments -PI,PI; integersinaprox ranges -127, 127 for arguments 0, 255
			// signal = 0.925*(0x01L<<(7+7))*Y + 0.075*255*(0x01L<<(7+7)) + 0.925*(0x01L<<(7))*sat*integersinaprox(hue_phase + position_phase);
			signal = 15154*Y + 313344 + 118*sat*integersinaprox(hue_phase + position_phase);
		}
		// signal = levelBlanking + signal * (levelWhite - levelBlanking + 1) / 256;
		signal = ((int32_t)(((int32_t)levelBlanking<<(8+7+7)) + signal * (levelWhite - levelBlanking + 1))) >> (8+7+7);
		if (signal > levelHighClipping) signal = levelHighClipping;
		if (signal < levelLowClipping) signal = levelLowClipping;
		return signal;
	}
	int buftocol(int val)
	{
		return 0;
	}
};
