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
#include "../Graphics/GraphicsR5G5B4S2Swapped.h"

class VGA14Bit : public VGA, public GraphicsR5G5B4S2Swapped
{
  public:
	VGA14Bit(const int i2sIndex = 1)
		: VGA(i2sIndex)
	{
	}

	bool init(Mode &mode,
			  const int R0Pin, const int R1Pin, const int R2Pin, const int R3Pin, const int R4Pin,
			  const int G0Pin, const int G1Pin, const int G2Pin, const int G3Pin, const int G4Pin,
			  const int B0Pin, const int B1Pin, const int B2Pin, const int B3Pin,
			  const int hsyncPin, const int vsyncPin, const int clockPin = -1)
	{
		int pinMap[16] = {
			R0Pin, R1Pin, R2Pin, R3Pin, R4Pin,
			G0Pin, G1Pin, G2Pin, G3Pin, G4Pin,
			B0Pin, B1Pin, B2Pin, B3Pin,
			hsyncPin, vsyncPin
		};

		return VGA::init(mode, pinMap, 16, clockPin);
	}

	bool init(const Mode &mode, const int *redPins, const int *greenPins, const int *bluePins, const int hsyncPin, const int vsyncPin, const int clockPin = -1)
	{
		int pinMap[16];
		for (int i = 0; i < 5; i++)
		{
			pinMap[i] = redPins[i];
			pinMap[i + 5] = greenPins[i];
			if (i < 4)
				pinMap[i + 10] = bluePins[i];
		}
		pinMap[14] = hsyncPin;
		pinMap[15] = vsyncPin;			
		return VGA::init(mode, pinMap, 16, clockPin);
	}

	bool init(const Mode &mode, const PinConfig &pinConfig)
	{
		int pins[16];
		pinConfig.fill14Bit(pins);
		return VGA::init(mode, pins, 16, pinConfig.clock);
	}

	virtual void initSyncBits()
	{
		hsyncBitI = mode.hSyncPolarity ? 0x4000 : 0;
		vsyncBitI = mode.vSyncPolarity ? 0x8000 : 0;
		hsyncBit = hsyncBitI ^ 0x4000;
		vsyncBit = vsyncBitI ^ 0x8000;
		SBits = hsyncBitI | vsyncBitI;
	}

	virtual long syncBits(bool hSync, bool vSync)
	{
		return ((hSync ? hsyncBit : hsyncBitI) | (vSync ? vsyncBit : vsyncBitI)) * 0x10001;
	}

	virtual int bytesPerSample() const
	{
		return 2;
	}

	virtual float pixelAspect() const
	{
		return 1;
	}

	virtual void propagateResolution(const int xres, const int yres)
	{
		setResolution(xres, yres);
	}

	virtual Color **allocateFrameBuffer()
	{
		return (Color **)DMABufferDescriptor::allocateDMABufferArray(yres, mode.hRes * bytesPerSample(), true, syncBits(false, false));
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
				dmaBufferDescriptors[(mode.vFront + mode.vSync + mode.vBack + i) * 2 + 1].setBuffer(frontBuffer[i / mode.vDiv], mode.hRes * bytesPerSample());
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