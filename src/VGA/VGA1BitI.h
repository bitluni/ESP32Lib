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
#include "../Graphics/GraphicsW1.h"

class VGA1BitI : public VGA, public GraphicsW1
{
  public:
	VGA1BitI()	//8 bit based modes only work with I2S1
		: VGA(1)
	{
		lineBufferCount = 3;
		interruptStaticChild = &VGA1BitI::interrupt;
	}

	bool init(const Mode &mode, const int RPin, const int GPin, const int BPin, const int hsyncPin, const int vsyncPin, const int clockPin = -1)
	{
		int pinMap[8] = {
			RPin,
			GPin,
			BPin,
			-1, -1, -1,
			hsyncPin, vsyncPin
		};
		return VGA::init(mode, pinMap, 8, clockPin);
	}

	bool init(const Mode &mode, const PinConfig &pinConfig)
	{
		int pins[8];
		pinConfig.fill3Bit(pins);
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
		//(bytesPerSample() == 1)
		for (int i = 0; i < samples; i++)
		{
			if (i < mode.hSync)
			{
				((unsigned char *)vSyncLineBuffer)[i ^ 2] = hsyncBit | vsyncBit;
				((unsigned char *)vBlankLineBuffer)[i ^ 2] = hsyncBit | vsyncBitI;
			}
			else
			{
				((unsigned char *)vSyncLineBuffer)[i ^ 2] = hsyncBitI | vsyncBit;
				((unsigned char *)vBlankLineBuffer)[i ^ 2] = hsyncBitI | vsyncBitI;
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


void IRAM_ATTR VGA1BitI::interrupt(void *arg)
{
	VGA1BitI * staticthis = (VGA1BitI *)arg;

	//fix for skipped lines due to skipped interupts during wifi activity
	DMABufferDescriptor *currentDmaBufferDescriptor = (DMABufferDescriptor *)REG_READ(I2S_OUT_EOF_DES_ADDR_REG(staticthis->i2sIndex));
	staticthis->dmaBufferDescriptorActive = ((uint32_t)currentDmaBufferDescriptor - (uint32_t)staticthis->dmaBufferDescriptors)/sizeof(DMABufferDescriptor);
	staticthis->currentLine = staticthis->dmaBufferDescriptorActive; //equivalent in this configuration
	
	int vInactiveLinesCount = staticthis->mode.vFront + staticthis->mode.vSync + staticthis->mode.vBack;
	
	//render ahead (the lenght of buffered lines)
	int renderLine = (staticthis->currentLine + staticthis->lineBufferCount) % staticthis->totalLines;
	
	if (renderLine >= vInactiveLinesCount)
	{
		int renderActiveLine = renderLine - vInactiveLinesCount;
		unsigned long *pixels = &((unsigned long *)staticthis->vActiveLineBuffer[renderActiveLine % staticthis->lineBufferCount])[(staticthis->mode.hSync + staticthis->mode.hBack) / 4];
		unsigned long base = (staticthis->hsyncBitI | staticthis->vsyncBitI) * 0x1010101;

		int y = renderActiveLine / staticthis->mode.vDiv;
		if (y >= 0 && y < staticthis->mode.vRes)
			staticthis->interruptPixelLine(y, pixels, base, arg);
	}

	if (renderLine == 0)
		staticthis->vSyncPassed = true;

	//update to provide currently outed buffer descriptor and line (increased by 1)
	//staticthis->currentLine = (staticthis->currentLine + 1) % staticthis->totalLines;
	//staticthis->dmaBufferDescriptorActive = (staticthis->dmaBufferDescriptorActive + 1) % staticthis->dmaBufferDescriptorCount;
}

void IRAM_ATTR VGA1BitI::interruptPixelLine(int y, unsigned long *pixels, unsigned long syncBits, void *arg)
{
	VGA1BitI * staticthis = (VGA1BitI *)arg;
	unsigned char *line = staticthis->frontBuffer[y];
	int j = 0;
	for (int i = 0; i < staticthis->mode.hRes / 4; i++)
	{
		j = i >> 1;
		uint32_t pixel = (line[j]>>(4*(1-(i & 1))));//&0xf;
		pixel = (pixel&(1<<3))<<13 | (pixel&(1<<2))<<22 | (pixel&(1<<1))>>1 | (pixel&(1<<0))<<8;
		pixels[i] = syncBits
		 | (pixel * staticthis->frontGlobalColor)
		 | ((pixel^0x01010101) * staticthis->backGlobalColor);
	}
}
