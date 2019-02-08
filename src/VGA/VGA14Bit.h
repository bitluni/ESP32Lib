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

	void *vSyncInactiveBuffer;
	void *vSyncActiveBuffer;
	void *inactiveBuffer;
	void *blankActiveBuffer;

	virtual Color** allocateFrameBuffer()
	{
		Color** frame = (Color **)malloc(yres * sizeof(Color *));
		for (int y = 0; y < yres; y++)
			frame[y] = (Color *)DMABufferDescriptor::allocateBuffer(hres * bytesPerSample, true, 0xc000c000);
		return frame;
	}

	virtual void allocateLineBuffers(const int lines)
	{
		dmaBufferDescriptorCount = totalLines * 2;
		int inactiveSamples = (hfront + hsync + hback + 1) & 0xfffffffe;
		vSyncInactiveBuffer = DMABufferDescriptor::allocateBuffer(inactiveSamples * bytesPerSample, true);
		vSyncActiveBuffer = DMABufferDescriptor::allocateBuffer(hres * bytesPerSample, true);
		inactiveBuffer = DMABufferDescriptor::allocateBuffer(inactiveSamples * bytesPerSample, true);
		blankActiveBuffer = DMABufferDescriptor::allocateBuffer(hres * bytesPerSample, true);
		for(int i = 0; i < inactiveSamples; i++)
		{
			if(i >= hfront && i < hfront + hsync)
			{
				((unsigned short*)vSyncInactiveBuffer)[i^1] = hsyncBit | vsyncBit;
				((unsigned short*)inactiveBuffer)[i^1] = hsyncBit | vsyncBitI;
			}
			else
			{
				((unsigned short*)vSyncInactiveBuffer)[i^1] = hsyncBitI | vsyncBit;
				((unsigned short*)inactiveBuffer)[i^1] = hsyncBitI | vsyncBitI;
			}
		}
		for(int i = 0; i < hres; i++)
		{
			((unsigned short*)vSyncActiveBuffer)[i^1] = hsyncBitI | vsyncBit;
			((unsigned short*)blankActiveBuffer)[i^1] = hsyncBitI | vsyncBitI;
		}

		dmaBufferDescriptors = DMABufferDescriptor::allocateDescriptors(dmaBufferDescriptorCount);
		for(int i = 0; i < dmaBufferDescriptorCount; i++)
			dmaBufferDescriptors[i].next(dmaBufferDescriptors[(i + 1) % dmaBufferDescriptorCount]);
		int d = 0;
		for (int i = 0; i < vfront; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(inactiveBuffer, inactiveSamples * bytesPerSample);
			dmaBufferDescriptors[d++].setBuffer(blankActiveBuffer, hres * bytesPerSample);
		}
		for (int i = 0; i < vsync; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(vSyncInactiveBuffer, inactiveSamples * bytesPerSample);
			dmaBufferDescriptors[d++].setBuffer(vSyncActiveBuffer, hres * bytesPerSample);
		}
		for (int i = 0; i < vback; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(inactiveBuffer, inactiveSamples * bytesPerSample);
			dmaBufferDescriptors[d++].setBuffer(blankActiveBuffer, hres * bytesPerSample);
		}
		for (int i = 0; i < yres * vdivider; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(inactiveBuffer, inactiveSamples * bytesPerSample);
			dmaBufferDescriptors[d++].setBuffer(frameBuffers[0][i / vdivider], hres * bytesPerSample);
		}
	}

	virtual void show()
	{
		if(!frameBufferCount)
			return;
		currentFrameBuffer = (currentFrameBuffer + 1) % frameBufferCount;
		frontBuffer = frameBuffers[currentFrameBuffer];
		backBuffer = frameBuffers[(currentFrameBuffer + frameBufferCount - 1) % frameBufferCount];
		for (int i = 0; i < yres * vdivider; i++)
			dmaBufferDescriptors[(vfront + vsync + vback + i) * 2 + 1].setBuffer(frontBuffer[i / vdivider], hres * bytesPerSample);
	}

	virtual void dotFast(int x, int y, Color color)
	{
		backBuffer[y][x^1] = color | 0xc000;
	}

	virtual void dot(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			backBuffer[y][x^1] = color | 0xc000;
	}

	virtual void dotAdd(int x, int y, Color color)
	{
		//todo repair this
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			backBuffer[y][x^1] = (color + backBuffer[y][x^1]) | 0xc000;
	}
	
	virtual void dotMix(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres && (color >> 14) != 0)
		{
			unsigned int ai = (3 - (color >> 14)) * (65536 / 3);
			unsigned int a = 65536 - ai;
			unsigned int co = backBuffer[y][x^1];
			unsigned int ro = (co & 0b11111) * ai;
			unsigned int go = (co & 0b1111100000) * ai;
			unsigned int bo = (co & 0b11110000000000) * ai;
			unsigned int r = (color & 0b11111) * a + ro;
			unsigned int g = ((color & 0b1111100000) * a + go) & 0b11111000000000000000000000;
			unsigned int b = ((color & 0b11110000000000) * a + bo) & 0b111100000000000000000000000000;
			backBuffer[y][x^1] = ((r | g | b) >> 16) | 0xc000;
		}	
	}
	
	virtual Color get(int x, int y)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			return backBuffer[y][x^1];
		return 0;
	}

	virtual void clear(Color clear = 0)
	{
		for (int y = 0; y < this->yres; y++)
			for (int x = 0; x < this->xres; x++)
				backBuffer[y][x^1] = clear | 0xc000;
	}

protected:
	virtual void interrupt()
	{
	}

};