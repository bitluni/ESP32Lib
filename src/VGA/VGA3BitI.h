/*
	Author: bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
		http://bitluni.net
*/
#pragma once
#include "VGA.h"
#include "../Graphics/GraphicsR1G1B1A1.h"

class VGA3BitI : public VGA, public GraphicsR1G1B1A1
{
  public:
	VGA3BitI(const int i2sIndex = 1)
		: VGA(i2sIndex)
	{
	}

	bool init(const Mode &mode, const int RPin, const int GPin, const int BPin, const int hsyncPin, const int vsyncPin)
	{
		int pinMap[24] = {
			-1, -1, -1, -1, -1, -1, -1, -1,
			RPin,
			GPin,
			BPin,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			hsyncPin, vsyncPin};
		return VGA::init(mode, pinMap);
	}

	virtual float pixelAspect() const
	{
		return 1;
	}

	virtual void propagateResolution(const int xres, const int yres)
	{
		setResolution(xres, yres);
	}

	virtual void show(bool vSync = false)
	{
		if (!frameBufferCount)
			return;
		if (vSync)
		{
			vSyncPassed = false;
			while (!vSyncPassed)
				delay(0);
		}
		Graphics::show(vSync);
	}

  protected:
	bool useInterrupt()
	{ 
		return true; 
	};

	void interruptPixelLine(int y, unsigned long *pixels, unsigned long syncBits)
	{
		unsigned char *line = frontBuffer[y];
		for (int i = 0; i < mode.hRes / 2; i++)
		{
			//writing two pixels improves speed drastically (avoids memory reads)
			pixels[i] = syncBits | ((line[i] >> 4) & 7) | ((line[i] & 7) << 16);
		}
	}
};
