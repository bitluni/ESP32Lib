/*
	Author: Martin-Laclaustra 2020 based on bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

class ColorR1G1B1A1X4
{
	public:
	typedef unsigned char Color;
	ColorR1G1B1A1X4() {}

	static const int static_colormask()
	{
		return 0b00000111;
	}

	static int static_R(Color c)
	{
		return (c & 1) * 255;
	}
	static int static_G(Color c)
	{
		return (c & 2) ? 255 : 0;
	}
	static int static_B(Color c)
	{
		return (c & 4) ? 255 : 0;
	}
	static int static_A(Color c)
	{
		return (c & 8) ? 255 : 0;
	}

	static Color static_RGBA(int r, int g, int b, int a = 255)
	{
		return ((r >> 7) & 1) | ((g >> 6) & 2) | ((b >> 5) & 4) | ((a >> 4) & 8);
	}

	static Color static_colorAdd(Color colorOld, Color colorNew)
	{
		return colorOld | colorNew;
	}

	static Color static_colorMix(Color colorOld, Color colorNew)
	{
		if ((colorNew & 8) != 0)
		{
			return colorNew;
		} else {
			return colorOld;
		}
	}
};
