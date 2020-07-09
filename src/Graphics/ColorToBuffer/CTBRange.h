/*
	Author: Martin-Laclaustra 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

class CTBRange
{
	public:
	CTBRange() {}

	int colorDepthConversionFactor = 256;
	int colorMinValue = 0;
	int colorMaxValue = 255;

	int coltobuf(int val, int /*x*/, int /*y*/)
	{
		return ((colorMinValue<<8) + colorDepthConversionFactor*val)>>8;
	}
	int buftocol(int val)
	{
		return ((val - colorMinValue)<<8) / colorDepthConversionFactor;
	}
};
