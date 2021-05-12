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
class BLpx4sz16swmx1yshmxy
{
	public:
	typedef unsigned short BufferUnit;

	static int mx;
	static int my;
	static int wx;
	static int wy;

	BLpx4sz16swmx1yshmxy() {}

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
		return 0b0000000000000111;
	}

	static const int static_replicate()
	{
		return 1;
	}
	static const int static_replicate32()
	{
		return 0x00010001;
	}

	static int static_swx(int x)
	{
		return (x % wx)^1;
	}
	static int static_swy(int y)
	{
		return y % wy;
	}
	static int static_shval(BufferUnit val, int x, int y)
	{
		return val<<(3*((x / wx + mx * (y / wy)) & 0x7));
	}
	static int static_shbuf(BufferUnit val, int x, int y)
	{
		return val>>(3*((x / wx + mx * (y / wy)) & 0x7));
	}
};

int BLpx4sz16swmx1yshmxy::wx;
int BLpx4sz16swmx1yshmxy::wy;
int BLpx4sz16swmx1yshmxy::mx;
int BLpx4sz16swmx1yshmxy::my;
