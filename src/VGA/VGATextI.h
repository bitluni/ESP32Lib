/*
	Author: Martin-Laclaustra 2021
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
*/
#pragma once
#include "VGAI2SEngine.h"
//#include "../Graphics/Graphics.h"
#include "../Graphics/GraphicsTextBuffer.h"

class VGATextI : public VGAI2SEngine<BLpx1sz8sw2sh0>, public GraphicsTextBuffer // (=) Graphics<ColorW8, BLpx1sz8sw0sh0, CTBIdentity>
{
  public:
	VGATextI() //8 bit based modes only work with I2S1
		: VGAI2SEngine<BLpx1sz8sw2sh0>(1)
	{
		frontColor = 0xf;
		interruptStaticChild = &VGATextI::interrupt;
	}

	bool init(const Mode &mode, const int RPin, const int GPin, const int BPin, const int hsyncPin, const int vsyncPin, const int clockPin = -1)
	{
		const int bitCount = 8;
		int pinMap[bitCount] = {
			RPin,
			GPin,
			BPin,
			-1, -1, -1,
			hsyncPin, vsyncPin
		};
		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

	bool init(const Mode &mode, const int *redPins, const int *greenPins, const int *bluePins, const int hsyncPin, const int vsyncPin, const int clockPin = -1, const bool mostSignigicantPinFirst = false)
	{
		const int bitCount = 8;
		int pinMap[bitCount];
		for (int i = 0; i < 8; i++)
		{
			pinMap[i] = -1;
		}
		pinMap[0] = redPins[0];
		pinMap[1] = greenPins[0];
		pinMap[2] = bluePins[0];
		pinMap[6] = hsyncPin;
		pinMap[7] = vsyncPin;

		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

	bool init(const Mode &mode, const PinConfig &pinConfig)
	{
		const int bitCount = 8;
		int pinMap[bitCount];
		pinConfig.fill3Bit(pinMap);
		int clockPin = pinConfig.clock;

		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

	bool inittext(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin, int descriptorsPerLine = 1)
	{
		if (!font)
			return false;
		this->mode = mode;
		int xres = (mode.hRes + font->charWidth - 1) / font->charWidth;
		int yres = ((mode.vRes / mode.vDiv) + font->charHeight - 1) / font->charHeight;
		initSyncBits();
		this->vsyncPin = vsyncPin;
		this->hsyncPin = hsyncPin;
		totalLines = mode.linesPerField();
		if(descriptorsPerLine < 1 || descriptorsPerLine > 2) ERROR("Wrong number of descriptors per line");
		if(descriptorsPerLine == 1) allocateRendererBuffers1DescriptorsPerLine();
		if(descriptorsPerLine == 2) allocateRendererBuffers2DescriptorsPerLine();
		propagateResolution(xres, yres);
		//allocateLineBuffers();
		currentLine = 0;
		vSyncPassed = false;
		initParallelOutputMode(pinMap, mode.pixelClock, bitCount, clockPin);
		startTX();
		return true;
	}

	static const int bitMaskInRenderingBufferHSync()
	{
		return 1<<(8*bytesPerBufferUnit()-2);
	}

	static const int bitMaskInRenderingBufferVSync()
	{
		return 1<<(8*bytesPerBufferUnit()-1);
	}

	bool initdynamicwritetorenderbuffer(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin = -1)
	{
		lineBufferCount = 3;
		rendererBufferCount = 1;
		return inittext(mode, pinMap, bitCount, clockPin, 1); // 1 buffer per line
	}

	//UPPER LIMIT: THE CODE BETWEEN THESE MARKS IS SHARED BETWEEN 3BIT, 6BIT, AND 14BIT

	virtual void initSyncBits()
	{
		hsyncBitI = mode.hSyncPolarity ? (bitMaskInRenderingBufferHSync()) : 0;
		vsyncBitI = mode.vSyncPolarity ? (bitMaskInRenderingBufferVSync()) : 0;
		hsyncBit = hsyncBitI ^ (bitMaskInRenderingBufferHSync());
		vsyncBit = vsyncBitI ^ (bitMaskInRenderingBufferVSync());
	}

	virtual long syncBits(bool hSync, bool vSync)
	{
		return ((hSync ? hsyncBit : hsyncBitI) | (vSync ? vsyncBit : vsyncBitI)) * rendererStaticReplicate32();
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

	static void interrupt(void *arg);

	static void interruptPixelLine(int y, uint8_t *pixels, void *arg);
};


void IRAM_ATTR VGATextI::interrupt(void *arg)
{
	VGATextI * staticthis = (VGATextI *)arg;

	//obtain currently rendered line from the buffer just read, based on the conventioned ordering and buffers per line
	staticthis->currentLine = staticthis->dmaBufferDescriptorActive >> ( (staticthis->descriptorsPerLine==2) ? 1 : 0 );

	//in the case of two buffers per line,
	//render only when the sync half of the line ended (longer period until next interrupt)
	//else exit early
	//This might need to be revised, because it might be better to overlap and miss the second interrupt
	if ( (staticthis->descriptorsPerLine==2) && ((staticthis->dmaBufferDescriptorActive & 1) != 0) ) return;

	//TO DO: This should be precalculated outside the interrupt
	int vInactiveLinesCount = staticthis->mode.vFront + staticthis->mode.vSync + staticthis->mode.vBack;

	//render ahead (the lenght of buffered lines)
	int renderLine = (staticthis->currentLine + staticthis->lineBufferCount);
	if (renderLine >= staticthis->totalLines) renderLine -= staticthis->totalLines;

	if (renderLine >= vInactiveLinesCount)
	{
		int renderActiveLine = renderLine - vInactiveLinesCount;
		uint8_t *activeRenderingBuffer = ((uint8_t *)
		staticthis->dmaBufferDescriptors[staticthis->indexRendererDataBuffer[0] + renderActiveLine * staticthis->descriptorsPerLine + staticthis->descriptorsPerLine - 1].buffer() + staticthis->dataOffsetInLineInBytes
		);

		int y = renderActiveLine / staticthis->mode.vDiv;
		if (y >= 0 && y < (staticthis->mode.vRes / staticthis->mode.vDiv) )
			staticthis->interruptPixelLine(y, activeRenderingBuffer, arg);
	}

	if (renderLine == 0)
		staticthis->vSyncPassed = true;
}

	//LOWER LIMIT: THE CODE BETWEEN THESE MARKS IS SHARED BETWEEN 3BIT, 6BIT, AND 14BIT

void IRAM_ATTR VGATextI::interruptPixelLine(int y, uint8_t *pixels8, void *arg)
{
	VGATextI * staticthis = (VGATextI *)arg;
	unsigned long syncBits = (staticthis->hsyncBitI | staticthis->vsyncBitI) * staticthis->rendererStaticReplicate32mask;
	uint32_t * pixels = (uint32_t *)pixels8;

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
