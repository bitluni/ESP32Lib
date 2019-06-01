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
#include "../Graphics/GraphicsR5G5B4A2.h"

class VGA14BitI : public VGA, public GraphicsR5G5B4A2
{
	public:
	VGA14BitI(const int i2sIndex = 1)
		: VGA(i2sIndex)
	{
	}

	bool init(const Mode &mode, 
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

	bool init(const Mode &mode, const PinConfig &pinConfig)
	{
		int pins[16];
		pinConfig.fill14Bit(pins);
		return VGA::init(mode, pins, 16, pinConfig.clock);
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

	virtual void initSyncBits()
	{
		hsyncBitI = mode.hSyncPolarity ? 0x4000 : 0;
		vsyncBitI = mode.vSyncPolarity ? 0x8000 : 0;
		hsyncBit = hsyncBitI ^ 0x4000;
		vsyncBit = vsyncBitI ^ 0x8000;
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
	virtual bool useInterrupt()
	{ 
		return true; 
	};

	void interruptPixelLine(int y, unsigned long *pixels, unsigned long syncBits)
	{
		unsigned short *line = frontBuffer[y];
		for (int i = 0; i < mode.hRes / 2; i++)
		{
			//writing two pixels improves speed drastically (avoids memory reads)
			pixels[i] = syncBits | (line[i * 2 + 1] & 0x3fff) | ((line[i * 2] & 0x3fff) << 16);
		}
	}
};