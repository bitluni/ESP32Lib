/*
	Author: bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 2.0
	https://creativecommons.org/licenses/by-sa/2.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
		http://bitluni.net
*/
#pragma once
#include "VGA.h"
#include "../Graphics/GraphicsR5G5B4A2.h"

class VGA14BitFast : public VGA, public GraphicsR5G5B4A2
{
	public:

	VGA14BitFast(const int i2sIndex = 1)
		: VGA(i2sIndex)
	{
		hsyncBit = 0x0000;
		vsyncBit = 0x0000;
		hsyncBitI = 0x4000;
		vsyncBitI = 0x8000;
	}

	bool init(const int *mode, 
		const int R0Pin, const int R1Pin, const int R2Pin, const int R3Pin, const int R4Pin,
		const int G0Pin, const int G1Pin, const int G2Pin, const int G3Pin, const int G4Pin,
		const int B0Pin, const int B1Pin, const int B2Pin, const int B3Pin, 
		const int hsyncPin, const int vsyncPin)
	{
		int pinMap[24] = {
			-1, -1, -1, -1, -1, -1, -1, -1,
			R0Pin, R1Pin, R2Pin, R3Pin, R4Pin,
			G0Pin, G1Pin, G2Pin, G3Pin, G4Pin,
			B0Pin, B1Pin, B2Pin, B3Pin,
			hsyncPin, vsyncPin
			};
		return VGA::init(mode, pinMap);
	}

	bool init(const int *mode, const int *redPins, const int *greenPins, const int *bluePins, const int hsyncPin, const int vsyncPin)
	{
		int pinMap[24];
		for (int i = 0; i < 8; i++)
			pinMap[i] = -1;
		for (int i = 0; i < 5; i++)
		{
			pinMap[i + 8] = redPins[i];
			pinMap[i + 13] = greenPins[i];
			if (i < 4)
				pinMap[i + 18] = bluePins[i];
		}
		pinMap[22] = hsyncPin;
		pinMap[23] = vsyncPin;
		return VGA::init(mode, pinMap);
	}

	virtual float pixelAspect() const
	{
		return float(vdivider) / hdivider;
	}

	virtual void propagateResolution(const int xres, const int yres)
	{
		setResolution(xres, yres);
	}
	
protected:
	virtual void interrupt()
	{
		unsigned long *signal = dmaBufferDescriptors[dmaBufferDescriptorActive]->buffer();
		unsigned long *pixels = &(dmaBufferDescriptors[dmaBufferDescriptorActive]->buffer())[(hfront + hsync + hback) / 2];
		unsigned long base, baseh;
		if (currentLine >= vfront && currentLine < vfront + vsync)
		{
			baseh = (vsyncBit | hsyncBit) | ((vsyncBit | hsyncBit) << 16);
			base = (vsyncBit | hsyncBitI) | ((vsyncBit | hsyncBitI) << 16);
		}
		else
		{
			baseh = (vsyncBitI | hsyncBit) | ((vsyncBitI | hsyncBit) << 16);
			base = (vsyncBitI | hsyncBitI) | ((vsyncBitI | hsyncBitI) << 16);
		}
		for (int i = 0; i < hfront / 2; i++)
			signal[i] = base;
		for (int i = hfront / 2; i < (hfront + hsync) / 2; i++)
			signal[i] = baseh;
		for (int i = (hfront + hsync) / 2; i < (hfront + hsync + hback) / 2; i++)
			signal[i] = base;

		int y = (currentLine - vfront - vsync - vback) / vdivider;
		if (y >= 0 && y < yres)
		{
			unsigned short *line = frontBuffer[y];
			for (int i = 0; i < xres / 2; i++)
			{
				//writing two pixels improves speed drastically (avoids reading in higher word)
				pixels[i] = base | (line[i * 2 + 1] & 0x3fff) | ((line[i * 2] & 0x3fff) << 16);
			}
		}
		else
			for (int i = 0; i < xres / 2; i++)
			{
				pixels[i] = base | (base << 16);
			}
		currentLine = (currentLine + 1) % totalLines;
		dmaBufferDescriptorActive = (dmaBufferDescriptorActive + 1) % dmaBufferDescriptorCount;
	}

};