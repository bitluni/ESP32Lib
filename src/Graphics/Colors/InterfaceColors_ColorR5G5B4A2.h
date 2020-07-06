/*
	Author: Martin-Laclaustra 2020 based on bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

class ColorR5G5B4A2
{
	public:
	typedef unsigned short Color;
	ColorR5G5B4A2() {}

	static const int static_colormask()
	{
		return 0b0011111111111111;
	}

	static int static_R(Color c)
	{
		return (((c << 1) & 0x3e) * 255 + 1) / 0x3e;
	}
	static int static_G(Color c)
	{
		return (((c >> 4) & 0x3e) * 255 + 1) / 0x3e;
	}
	static int static_B(Color c)
	{
		return (((c >> 9) & 0x1e) * 255 + 1) / 0x1e;
	}
	static int static_A(Color c)
	{
		return (((c >> 13) & 6) * 255 + 1) / 6;
	}

	static Color static_RGBA(int r, int g, int b, int a = 255)
	{
		return ((r >> 3) & 0b11111) | ((g << 2) & 0b1111100000) | ((b << 6) & 0b11110000000000) | ((a << 8) & 0xc000);
	}

	static Color static_colorAdd(Color colorOld, Color colorNew)
	{
		int c0 = colorOld;
		int c1 = colorNew;
		int r = (c0 & 0b11111) + (c1 & 0b11111);
		if(r > 0b11111) r = 0b11111;
		int g = (c0 & 0b1111100000) + (c1 & 0b1111100000);
		if(g > 0b1111100000) g = 0b1111100000;
		int b = (c0 & 0b11110000000000) + (c1 & 0b11110000000000);
		if(b > 0b11110000000000) b = 0b11110000000000;
		return r | (g & 0b1111100000) | (b & 0b11110000000000);
	}

	static Color static_colorMix(Color colorOld, Color colorNew)
	{
		if ((colorNew >> 14) != 0)
		{
			unsigned int ai = (3 - (colorNew >> 14)) * (65536 / 3); // consider to remove parenthesis
			unsigned int a = 65536 - ai;
			unsigned int ro = (colorOld & 0b11111) * ai;
			unsigned int go = (colorOld & 0b1111100000) * ai;
			unsigned int bo = (colorOld & 0b11110000000000) * ai;
			unsigned int r = (colorNew & 0b11111) * a + ro;
			unsigned int g = ((colorNew & 0b1111100000) * a + go) & 0b11111000000000000000000000;
			unsigned int b = ((colorNew & 0b11110000000000) * a + bo) & 0b111100000000000000000000000000;
			return (r | g | b) >> 16;
		} else {
			return colorOld;
		}
	}
};
