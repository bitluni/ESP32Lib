/*
	Author: Martin-Laclaustra 2020
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
class BLpx4sz8sw3xshx
{
	public:
	typedef unsigned char BufferUnit;
	BLpx4sz8sw3xshx() {}

	static const int static_xpixperunit()
	{
		return 4;
	}

	static const int static_ypixperunit()
	{
		return 1;
	}

	static const int static_bufferdatamask()
	{
		return 0b00000011;
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
		return (x>>2)^3;
	}
	static int static_swy(int y)
	{
		return y;
	}
	static int static_shval(BufferUnit val, int x, int /*y*/)
	{
		return val<<(2*((x&3)^3));
	}
	static int static_shbuf(BufferUnit val, int x, int /*y*/)
	{
		return val>>(2*((x&3)^3));
	}
};
