/*
	Author: Martin-Laclaustra 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

class CTBRangePDM8
{
	public:
	CTBRangePDM8() {}

	int colorDepthConversionFactor = 256;
	int colorMinValue = 0;
	int colorMaxValue = 255;

	uint8_t valToPDMLUT[8] = {
		0b00000000,
//		0b00000001,
		0b00010001,
		0b00100101,
		0b01010101,
		0b01011011,
		0b01110111,
		0b01111111,
		0b11111111,
	};

	int coltobuf(int val, int /*x*/, int /*y*/)
	{
		return valToPDMLUT[((colorMinValue<<8) + colorDepthConversionFactor*val)>>8>>5];
	}
	int buftocol(int val)
	{
		int valsetbitcount = (val >> 0) & 1 +
							(val >> 1) & 1 +
							(val >> 2) & 1 +
							(val >> 3) & 1 +
							(val >> 4) & 1 +
							(val >> 5) & 1 +
							(val >> 6) & 1 +
							(val >> 7) & 1;
		return (((colorMinValue>(valsetbitcount<<5))?0:(valsetbitcount<<5 - colorMinValue))<<8) / colorDepthConversionFactor;
	}
};
