/*
	Author: Martin-Laclaustra 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

class CTBRangePDM2
{
	public:
	CTBRangePDM2() {}

	int colorDepthConversionFactor = 256;
	int colorMinValue = 0;
	int colorMaxValue = 255;

	uint8_t valToPDMLUT[3] = {
		0b00000000,
		0b00000010,
		0b00000011,
	};

	int coltobuf(int val, int /*x*/, int /*y*/)
	{
		//return valToPDMLUT[(((colorMinValue<<8) + colorDepthConversionFactor*val)>>8)>>6];
		return valToPDMLUT[(val>0)?2:1];
	}
	int buftocol(int val)
	{
		//return (((colorMinValue>(val<<6))?0:((val<<6) - colorMinValue))<<8) / colorDepthConversionFactor;
		return (val>2)?255:0;
	}
};
