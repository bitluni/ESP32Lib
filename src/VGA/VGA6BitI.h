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
#include "../Graphics/GraphicsR2G2B2A2.h"

class VGA6BitI : public VGA, public GraphicsR2G2B2A2
{
  public:
	VGA6BitI()	//8 bit based modes only work with I2S1
		: VGA(1)
	{
	}

	bool init(Mode &mode,
			  const int R0Pin, const int R1Pin,
			  const int G0Pin, const int G1Pin,
			  const int B0Pin, const int B1Pin,
			  const int hsyncPin, const int vsyncPin, const int clockPin = -1)
	{
		int pinMap[8] = {
			R0Pin, R1Pin,
			G0Pin, G1Pin,
			B0Pin, B1Pin,
			hsyncPin, vsyncPin
		};

		return VGA::init(mode, pinMap, 8, clockPin);
	}

	bool init(const Mode &mode, const int *redPins, const int *greenPins, const int *bluePins, const int hsyncPin, const int vsyncPin, const int clockPin = -1)
	{
		int pinMap[8];
		for (int i = 0; i < 2; i++)
		{
			pinMap[i] = redPins[i];
			pinMap[i + 2] = greenPins[i];
			pinMap[i + 4] = bluePins[i];
		}
		pinMap[6] = hsyncPin;
		pinMap[7] = vsyncPin;			
		return VGA::init(mode, pinMap, 8, clockPin);
	}

	bool init(const Mode &mode, const PinConfig &pinConfig)
	{
		int pins[8];
		pinConfig.fill6Bit(pins);
		return VGA::init(mode, pins, 8, pinConfig.clock);
	}

	virtual void initSyncBits()
	{
		hsyncBitI = mode.hSyncPolarity ? 0x40 : 0;
		vsyncBitI = mode.vSyncPolarity ? 0x80 : 0;
		hsyncBit = hsyncBitI ^ 0x40;
		vsyncBit = vsyncBitI ^ 0x80;
	}

	virtual long syncBits(bool hSync, bool vSync)
	{
		return ((hSync ? hsyncBit : hsyncBitI) | (vSync ? vsyncBit : vsyncBitI)) * 0x1010101;
	}

	virtual int bytesPerSample() const
	{
		return 1;
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

	void interrupt()
	{
		unsigned long *signal = (unsigned long *)dmaBufferDescriptors[dmaBufferDescriptorActive].buffer();
		unsigned long *pixels = &((unsigned long *)dmaBufferDescriptors[dmaBufferDescriptorActive].buffer())[(mode.hSync + mode.hBack) / 4];
		unsigned long base, baseh;
		if (currentLine >= mode.vFront && currentLine < mode.vFront + mode.vSync)
		{
			baseh = syncBits(true, true);
			base = syncBits(false, true);
		}
		else
		{
			baseh = syncBits(true, false);
			base =  syncBits(false, false);
		}
		for (int i = 0; i < mode.hSync / 4; i++)
			signal[i] = baseh;
		for (int i = mode.hSync / 4; i < (mode.hSync + mode.hBack) / 4; i++)
			signal[i] = base;

		int y = (currentLine - mode.vFront - mode.vSync - mode.vBack) / mode.vDiv;
		if (y >= 0 && y < mode.vRes)
			interruptPixelLine(y, pixels, base);
		else
			for (int i = 0; i < mode.hRes / 4; i++)
			{
				pixels[i] = base;
			}
		for (int i = 0; i < mode.hFront / 4; i++)
			signal[i + (mode.hSync + mode.hBack + mode.hRes) / 4] = base;
		currentLine = (currentLine + 1) % totalLines;
		dmaBufferDescriptorActive = (dmaBufferDescriptorActive + 1) % dmaBufferDescriptorCount;
		if (currentLine == 0)
			vSync();
	}

	void interruptPixelLine(int y, unsigned long *pixels, unsigned long syncBits)
	{
		unsigned char *line = frontBuffer[y];
		int j = 0;
		for (int i = 0; i < mode.hRes / 4; i++)
		{
			int p0 = (line[j] >> 0) & 7;
			int p1 = (line[j++] >> 4) & 7;
			int p2 = (line[j] >> 0) & 7;
			int p3 = (line[j++] >> 4) & 7;
			pixels[i] = syncBits | (p2 << 0) | (p3 << 8) | (p0 << 16) | (p1 << 24);
		}
	}
};
