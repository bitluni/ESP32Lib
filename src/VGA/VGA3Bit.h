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
#include "../Graphics/GraphicsR1G1B1A1X10S2Swapped.h"

class VGA3Bit : public VGA, public GraphicsR5G5B4A1X10S2Swapped
{
  public:
	VGA3Bit(const int i2sIndex = 1)
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

		hsyncBitI = mode.hSyncPolarity ? 0x4000 : 0;
		vsyncBitI = mode.vSyncPolarity ? 0x8000 : 0;
		hsyncBit = hsyncBitI ^ 0x4000;
		vsyncBit = vsyncBitI ^ 0x8000;
		SBits = hsyncBitI | vsyncBitI;

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

	void *vSyncInactiveBuffer;
	void *vSyncActiveBuffer;
	void *inactiveBuffer;
	void *blankActiveBuffer;

	virtual Color **allocateFrameBuffer()
	{
		return (Color **)DMABufferDescriptor::allocateDMABufferArray(yres, mode.hRes * bytesPerSample, true, (hsyncBitI | vsyncBitI) * 0x10001);
	}

	virtual void allocateLineBuffers()
	{
		VGA::allocateLineBuffers((void **)frameBuffers[0]);
	}

	virtual void show(bool vSync = false)
	{
		if (!frameBufferCount)
			return;
		if (vSync)
		{
			//TODO read the I2S docs to find out
		}
		Graphics::show(vSync);
		if(dmaBufferDescriptors)
		for (int i = 0; i < yres * mode.vDiv; i++)
			dmaBufferDescriptors[(mode.vFront + mode.vSync + mode.vBack + i) * 2 + 1].setBuffer(frontBuffer[i / mode.vDiv], mode.hRes * bytesPerSample);
	}

	virtual void scroll(int dy, Color color)
	{
		Graphics::scroll(dy, color);
		if (frameBufferCount == 1)
			show();
	}

  protected:
	virtual void interrupt()
	{
	}
};
