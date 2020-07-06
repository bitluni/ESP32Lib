/*
	Author: Martin-Laclaustra 2020 based on bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

class ColorW1X7
{
	public:
	typedef unsigned char Color;
	ColorW1X7() {}

	static const int static_colormask()
	{
		return 0b00000001;
	}

	static int static_R(Color c)
	{
		return (c & 1) ? 255 : 0;
	}
	static int static_G(Color c)
	{
		return (c & 1) ? 255 : 0;
	}
	static int static_B(Color c)
	{
		return (c & 1) ? 255 : 0;
	}
	static int static_A(Color c)
	{
		return (c & 1) ? 255 : 0;
	}

	static Color static_RGBA(int r, int g, int b, int a = 255)
	{
		return (((r > 0)?1:0) | ((g > 0)?1:0) | ((b > 0)?1:0)) & ((a > 0)?1:0);
	}

	static Color static_colorAdd(Color colorOld, Color colorNew)
	{
		return colorOld | colorNew;
	}
	
	static Color static_colorMix(Color colorOld, Color colorNew)
	{
		return colorOld | colorNew;
	}
};
