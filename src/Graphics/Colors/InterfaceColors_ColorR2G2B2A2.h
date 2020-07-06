/*
	Author: Martin-Laclaustra 2020 based on bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

class ColorR2G2B2A2
{
	public:
	typedef unsigned char Color;
	ColorR2G2B2A2() {}

	static const int static_colormask()
	{
		return 0b00111111;
	}

	static int static_R(Color c)
	{
		return (((int)c & 3) * 255 + 1) / 3;
	}
	static int static_G(Color c)
	{
		return (((int)(c >> 2) & 3) * 255 + 1) / 3;
	}
	static int static_B(Color c)
	{
		return (((int)(c >> 4) & 3) * 255 + 1) / 3;
	}
	static int static_A(Color c)
	{
		return (((int)(c >> 6) & 3) * 255 + 1) / 3;
	}

	static Color static_RGBA(int r, int g, int b, int a = 255)
	{
		return ((r >> 6) & 0b11) | ((g >> 4) & 0b1100) | ((b >> 2) & 0b110000) | (a & 0b11000000);
	}

	static Color static_colorAdd(Color colorOld, Color colorNew)
	{
		int c0 = colorOld;
		int c1 = colorNew;
		int r = (c0 & 0b11) + (c1 & 0b11);
		if(r > 0b11) r = 0b11;
		int g = (c0 & 0b1100) + (c1 & 0b1100);
		if(g > 0b1100) g = 0b1100;
		int b = (c0 & 0b110000) + (c1 & 0b110000);
		if(b > 0b110000) b = 0b110000;
		return r | (g & 0b1100) | (b & 0b110000);
	}

	static Color static_colorMix(Color colorOld, Color colorNew)
	{
		if ((colorNew >> 6) != 0)
		{
			unsigned int ai = (3 - ((int)colorNew >> 6)) * (65536 / 3); // consider to remove parenthesis
			unsigned int a = 65536 - ai;
			unsigned int ro = (colorOld & 0b11) * ai;
			unsigned int go = (colorOld & 0b1100) * ai;
			unsigned int bo = (colorOld & 0b110000) * ai;
			unsigned int r = (colorNew & 0b11) * a + ro;
			unsigned int g = ((colorNew & 0b1100) * a + go) & 0b11000000000000000000;
			unsigned int b = ((colorNew & 0b110000) * a + bo) & 0b1100000000000000000000;
			return (r | g | b) >> 16;
		} else {
			return colorOld;
		}
	}
};
