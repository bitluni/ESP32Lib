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
#include "../Graphics/GraphicsTextBuffer.h"

class VGATextI : public VGA, public GraphicsTextBuffer
{
  public:
	VGATextI()	//8 bit based modes only work with I2S1
		: VGA(1)
	{
		lineBufferCount = 3;
		interruptStaticChild = &VGATextI::interrupt;
	}

	bool textinit(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin)
	{
		if (!font)
			return false;
		this->mode = mode;
		int xres = (mode.hRes + font->charWidth - 1) / font->charWidth;
		int yres = ((mode.vRes / mode.vDiv) + font->charHeight - 1) / font->charHeight;
		initSyncBits();
		propagateResolution(xres, yres);
		this->vsyncPin = vsyncPin;
		this->hsyncPin = hsyncPin;
		totalLines = mode.linesPerField();
		allocateLineBuffers();
		currentLine = 0;
		vSyncPassed = false;
		initParallelOutputMode(pinMap, mode.pixelClock, bitCount, clockPin);
		startTX();
		return true;
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
		return textinit(mode, pinMap, 8, clockPin);
	}

	bool init(const Mode &mode, const PinConfig &pinConfig)
	{
		int pins[8];
		pinConfig.fill3Bit(pins);
		return textinit(mode, pins, 8, pinConfig.clock);
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


void IRAM_ATTR VGATextI::interrupt(void *arg)
{
	VGATextI * staticthis = (VGATextI *)arg;

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

void IRAM_ATTR VGATextI::interruptPixelLine(int y, unsigned long *pixels, unsigned long syncBits, void *arg)
{
	VGATextI * staticthis = (VGATextI *)arg;
	unsigned char *charline = staticthis->frontBuffer[y/staticthis->font->charHeight];
 	unsigned char charactersubline = y % staticthis->font->charHeight;

	uint32_t pixel = 0;
	uint8_t * pixelbyte = (uint8_t *)&pixel;
	unsigned char renderchar = 0;
	const uint8_t *line = &staticthis->font->pixels[staticthis->font->charWidth * charactersubline];
	const uint8_t *lineoffset = 0;
	const uint8_t characterinterval = staticthis->font->charWidth * staticthis->font->charHeight;


	//pixel in row
	int i = 0;
	//which pixel column within the character
	int xcolumn = 0;
	renderchar = *charline;
	if (!staticthis->font->valid(renderchar)) renderchar = ' ';
	lineoffset = &line[xcolumn + characterinterval * (renderchar - staticthis->font->firstChar)];
	while (i < staticthis->mode.hRes)
	{
		if(staticthis->font->charWidth - xcolumn >=4)
		{
			pixelbyte[2] = *lineoffset++;
			pixelbyte[3] = *lineoffset++;
			pixelbyte[0] = *lineoffset++;
			pixelbyte[1] = *lineoffset++;
			if((xcolumn += 4) >= staticthis->font->charWidth)
			{
				xcolumn=0;
				renderchar = *++charline;
				if (!staticthis->font->valid(renderchar)) renderchar = ' ';
				lineoffset = &line[characterinterval * (renderchar - staticthis->font->firstChar)];
			}
		} else {
			switch(staticthis->font->charWidth - xcolumn)
			{
				case 1:
					pixelbyte[2] = *lineoffset++;
					renderchar = *++charline;
					if (!staticthis->font->valid(renderchar)) renderchar = ' ';
					lineoffset = &line[characterinterval * (renderchar - staticthis->font->firstChar)];
					pixelbyte[3] = *lineoffset++;
					pixelbyte[0] = *lineoffset++;
					pixelbyte[1] = *lineoffset++;
					xcolumn=3;
					break;
				case 2:
					pixelbyte[2] = *lineoffset++;
					pixelbyte[3] = *lineoffset++;
					renderchar = *++charline;
					if (!staticthis->font->valid(renderchar)) renderchar = ' ';
					lineoffset = &line[characterinterval * (renderchar - staticthis->font->firstChar)];
					pixelbyte[0] = *lineoffset++;
					pixelbyte[1] = *lineoffset++;
					xcolumn=2;
					break;
				case 3:
					pixelbyte[2] = *lineoffset++;
					pixelbyte[3] = *lineoffset++;
					pixelbyte[0] = *lineoffset++;
					renderchar = *++charline;
					if (!staticthis->font->valid(renderchar)) renderchar = ' ';
					lineoffset = &line[characterinterval * (renderchar - staticthis->font->firstChar)];
					pixelbyte[1] = *lineoffset++;
					xcolumn=1;
					break;
			}
		}

		pixel &= 0x01010101;
		*pixels++ = syncBits
		 | (pixel * staticthis->frontGlobalColor)
		 | ((pixel^0x01010101) * staticthis->backGlobalColor);
		i+=4;
	}
}

