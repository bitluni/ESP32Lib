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
class BLpx2sz8swxshx
{
	public:
	typedef unsigned char BufferUnit;
	BLpx2sz8swxshx() {}

	static const int static_xpixperunit()
	{
		return 2;
	}

	static const int static_ypixperunit()
	{
		return 1;
	}

	static const int static_bufferdatamask()
	{
		return 0b00001111;
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
		return x>>1;
	}
	static int static_swy(int y)
	{
		return y;
	}
	static int static_shval(BufferUnit val, int x, int /*y*/)
	{
		return val<<((x&1)<<2);
	}
	static int static_shbuf(BufferUnit val, int x, int /*y*/)
	{
		return val>>((x&1)<<2);
	}
};
