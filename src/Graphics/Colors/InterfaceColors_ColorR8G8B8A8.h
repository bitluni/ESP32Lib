/*
	Author: Martin-Laclaustra 2020 based on bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

class ColorR8G8B8A8
{
	public:
	typedef unsigned int Color;
	ColorR8G8B8A8() {}

	static const int static_colormask()
	{
		return 0x00ffffff;
	}

	static int static_R(Color c)
	{
		return c & 0xff;
	}
	static int static_G(Color c)
	{
		return (c >> 8) & 0xff;
	}
	static int static_B(Color c)
	{
		return (c >> 16) & 0xff;
	}
	static int static_A(Color c)
	{
		return (c >> 24) & 0xff;
	}

	static Color static_RGBA(int r, int g, int b, int a = 255)
	{
		return (r & 0x000000ff) | ((g << 8) & 0x0000ff00) | ((b << 16) & 0x00ff0000) | ((a << 24) & 0xff000000);
	}

	static Color static_colorAdd(Color colorOld, Color colorNew)
	{
		int c0 = colorOld;
		int c1 = colorNew;
		int r = (c0 & 0x000000ff) + (c1 & 0x000000ff);
		if(r > 0x000000ff) r = 0x000000ff;
		int g = (c0 & 0x0000ff00) + (c1 & 0x0000ff00);
		if(g > 0x0000ff00) g = 0x0000ff00;
		int b = (c0 & 0x00ff0000) + (c1 & 0x00ff0000);
		if(b > 0x00ff0000) b = 0x00ff0000;
		return r | (g & 0x0000ff00) | (b & 0x00ff0000);
	}

	static Color static_colorMix(Color colorOld, Color colorNew)
	{
		if ((colorNew >> 24) != 0)
		{
			uint64_t ai = (3 - (colorNew >> 24)) * ((0xffffffffLL + 1) / 3); // consider to remove parenthesis
			uint64_t a = (0xffffffffLL + 1) - ai;
			uint64_t ro = (colorOld & 0x000000ff) * ai;
			uint64_t go = (colorOld & 0x0000ff00) * ai;
			uint64_t bo = (colorOld & 0x00ff0000) * ai;
			uint64_t r = ((colorNew & 0x000000ff) * a + ro) & 0x000000ff00000000LL;
			uint64_t g = ((colorNew & 0x0000ff00) * a + go) & 0x0000ff0000000000LL;
			uint64_t b = ((colorNew & 0x00ff0000) * a + bo) & 0x00ff000000000000LL;
			return (Color)((r | g | b) >> 32);
		} else {
			return colorOld;
		}
	}
};
