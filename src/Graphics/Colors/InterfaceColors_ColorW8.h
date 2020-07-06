/*
	Author: Martin-Laclaustra 2020 based on bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

class ColorW8
{
	public:
	typedef unsigned char Color;
	ColorW8() {}

	static const int static_colormask()
	{
		return 0b11111111;
	}

	static int static_R(Color c)
	{
		return c;
	}
	static int static_G(Color c)
	{
		return c;
	}
	static int static_B(Color c)
	{
		return c;
	}
	static int static_A(Color c)
	{
		return 255;
	}

	static Color static_RGBA(int r, int g, int b, int a = 255)
	{
		//return (r * 2126 + g * 7152 + b * 722) / 10000;
		// binary weigthed mean:
		return (r * 13933 + g * 46871 + b * 4732) >> 16;
	}

	static Color static_colorAdd(Color colorOld, Color colorNew)
	{
		int sumColor = (int)colorOld + colorNew;
		return (sumColor > 0xff) ? 0xff : sumColor;
	}

	static Color static_colorMix(Color colorOld, Color colorNew)
	{
		return ((int)colorOld + colorNew) >> 1;
	}
};
