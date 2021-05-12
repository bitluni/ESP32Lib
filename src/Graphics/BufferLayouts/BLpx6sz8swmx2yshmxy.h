/*
	Author: Martin-Laclaustra 2021
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

// px pixels per unit
// sz unit size in bits
// sw nor swap or shift of x or y
// sh shift bits within unit
class BLpx6sz8swmx2yshmxy
{
	public:
	typedef unsigned char BufferUnit;

	static int mx;
	static int my;
	static int wx;
	static int wy;

	BLpx6sz8swmx2yshmxy() {}

	static const int static_xpixperunit()
	{
		return 1;
	}

	static const int static_ypixperunit()
	{
		return mx*my;
	}

	static const int static_bufferdatamask()
	{
		return 0b00000001;
	}

	static const int static_replicate()
	{
		return 1;
	}
	static const int static_replicate32()
	{
		return 0x01010101;
	}

	static int static_swx(int x)
	{
		return (x % wx)^2;
	}
	static int static_swy(int y)
	{
		return y % wy;
	}
	static int static_shval(BufferUnit val, int x, int y)
	{
		return val<<(((x / wx + mx * (y / wy)) & 0x7));
	}
	static int static_shbuf(BufferUnit val, int x, int y)
	{
		return val>>(((x / wx + mx * (y / wy)) & 0x7));
	}
};

int BLpx6sz8swmx2yshmxy::wx;
int BLpx6sz8swmx2yshmxy::wy;
int BLpx6sz8swmx2yshmxy::mx;
int BLpx6sz8swmx2yshmxy::my;
