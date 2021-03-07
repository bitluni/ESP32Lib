/*
	Author: Martin-Laclaustra 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

class CTBRangePDM4
{
	public:
	CTBRangePDM4() {}

	int colorDepthConversionFactor = 256;
	int colorMinValue = 0;
	int colorMaxValue = 255;

	uint8_t valToPDMLUT[4] = {
		0b00000000,
//		0b00000001,
		0b00000101,
		0b00000111,
		0b00001111,
	};

	int coltobuf(int val, int /*x*/, int /*y*/)
	{
		return valToPDMLUT[(((colorMinValue<<8) + colorDepthConversionFactor*val)>>8)>>6];
	}
	int buftocol(int val)
	{
		int valsetbitcount = (val >> 0) & 1 +
							(val >> 1) & 1 +
							(val >> 2) & 1 +
							(val >> 3) & 1;
		return (((colorMinValue>(valsetbitcount<<6))?0:((valsetbitcount<<6) - colorMinValue))<<8) / colorDepthConversionFactor;
	}
};
