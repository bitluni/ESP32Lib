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
		lineBufferCount = 3;
		interruptStaticChild = &VGA14BitI::interrupt;
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

	void *vBlankLineBuffer;
	void *vSyncLineBuffer;
	void **vActiveLineBuffer;

	//complete ring of buffer descriptors for one frame
	//actual linebuffers only for some lines rendered ahead
	virtual void allocateLineBuffers()
	{
		//lenght of each line
		int samples = mode.hFront + mode.hSync + mode.hBack + mode.hRes;
		int bytes = samples * bytesPerSample();

		//create and fill the buffers with their default values

		//create the buffers
		//1 blank prototype line for vFront and vBack
		vBlankLineBuffer = DMABufferDescriptor::allocateBuffer(bytes, true);
		//1 sync prototype line for vSync
		vSyncLineBuffer = DMABufferDescriptor::allocateBuffer(bytes, true);
		//n lines as buffer for active lines
		int ActiveLinesBufferCount = lineBufferCount;
		vActiveLineBuffer = (void **)malloc(ActiveLinesBufferCount * sizeof(void *));
		if(!vActiveLineBuffer)
			ERROR("Not enough memory for ActiveLineBuffer buffer");
		for (int i = 0; i < ActiveLinesBufferCount; i++)
		{
			vActiveLineBuffer[i] = DMABufferDescriptor::allocateBuffer(bytes, true);
		}

		//fill the buffers with their default values
		//(bytesPerSample() == 2)
		for (int i = 0; i < samples; i++)
		{
			if (i < mode.hSync)
			{
				((unsigned short *)vSyncLineBuffer)[i ^ 1] = hsyncBit | vsyncBit;
				((unsigned short *)vBlankLineBuffer)[i ^ 1] = hsyncBit | vsyncBitI;
			}
			else
			{
				((unsigned short *)vSyncLineBuffer)[i ^ 1] = hsyncBitI | vsyncBit;
				((unsigned short *)vBlankLineBuffer)[i ^ 1] = hsyncBitI | vsyncBitI;
			}
		}
		for (int i = 0; i < ActiveLinesBufferCount; i++)
		{
			memcpy(vActiveLineBuffer[i], vBlankLineBuffer, bytes);
		}

		//allocate DMA buffer descriptors for the whole frame
		dmaBufferDescriptorCount = totalLines;
		dmaBufferDescriptors = DMABufferDescriptor::allocateDescriptors(dmaBufferDescriptorCount);
		//link all buffer descriptors in a ring
		for (int i = 0; i < dmaBufferDescriptorCount; i++)
			dmaBufferDescriptors[i].next(dmaBufferDescriptors[(i + 1) % dmaBufferDescriptorCount]);

		//assign the buffers accross the DMA buffer descriptors
		//CONVENTION: the frame starts after the last active line of previous frame
		int d = 0;
		for (int i = 0; i < mode.vFront; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(vBlankLineBuffer, bytes);
		}
		for (int i = 0; i < mode.vSync; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(vSyncLineBuffer, bytes);
		}
		for (int i = 0; i < mode.vBack; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(vBlankLineBuffer, bytes);
		}
		for (int i = 0; i < mode.vRes; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(vActiveLineBuffer[i % ActiveLinesBufferCount], bytes);
		}
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

	static void interrupt(void *arg);

	static void interruptPixelLine(int y, unsigned long *pixels, unsigned long syncBits, void *arg);
};


void IRAM_ATTR VGA14BitI::interrupt(void *arg)
{
	VGA14BitI * staticthis = (VGA14BitI *)arg;

	staticthis->currentLine = staticthis->dmaBufferDescriptorActive; //equivalent in this configuration
	
	int vInactiveLinesCount = staticthis->mode.vFront + staticthis->mode.vSync + staticthis->mode.vBack;
	
	//render ahead (the lenght of buffered lines)
	int renderLine = (staticthis->currentLine + staticthis->lineBufferCount) % staticthis->totalLines;
	
	if (renderLine >= vInactiveLinesCount)
	{
		int renderActiveLine = renderLine - vInactiveLinesCount;
		unsigned long *pixels = &((unsigned long *)staticthis->vActiveLineBuffer[renderActiveLine % staticthis->lineBufferCount])[(staticthis->mode.hSync + staticthis->mode.hBack) / 2];
		unsigned long base = (staticthis->hsyncBitI | staticthis->vsyncBitI) * 0x10001;

		int y = renderActiveLine / staticthis->mode.vDiv;
		if (y >= 0 && y < staticthis->mode.vRes)
			staticthis->interruptPixelLine(y, pixels, base, arg);
	}

	if (renderLine == 0)
		staticthis->vSyncPassed = true;
}

void IRAM_ATTR VGA14BitI::interruptPixelLine(int y, unsigned long *pixels, unsigned long syncBits, void *arg)
{
	VGA14BitI * staticthis = (VGA14BitI *)arg;
	unsigned short *line = staticthis->frontBuffer[y];
	for (int i = 0; i < staticthis->mode.hRes / 2; i++)
	{
		//writing two pixels improves speed drastically (avoids memory reads)
		pixels[i] = syncBits | (line[i * 2 + 1] & 0x3fff) | ((line[i * 2] & 0x3fff) << 16);
	}
}
