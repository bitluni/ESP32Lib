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
#include "VGAI2SEngine.h"
//#include "../Graphics/Graphics.h"
#include "../Graphics/GraphicsR1G1B1A1.h"

class VGA3BitI : public VGAI2SEngine<BLpx1sz8sw2sh0>, public GraphicsR1G1B1A1 // (=) public Graphics<ColorR1G1B1A1X4, BLpx2sz8swxshx, CTBIdentity>
{
  public:
	VGA3BitI() //8 bit based modes only work with I2S1
		: VGAI2SEngine<BLpx1sz8sw2sh0>(1)
	{
		frontColor = 0xf;
		interruptStaticChild = &VGA3BitI::interrupt;
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

	//UPPER LIMIT: THE CODE BETWEEN THESE MARKS IS SHARED BETWEEN 3BIT, 6BIT, AND 14BIT

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
		return initengine(mode, pinMap, bitCount, clockPin, 1); // 1 buffer per line
	}

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


void IRAM_ATTR VGA3BitI::interrupt(void *arg)
{
	VGA3BitI * staticthis = (VGA3BitI *)arg;

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
		if (y >= 0 && y < staticthis->yres)
			staticthis->interruptPixelLine(y, activeRenderingBuffer, arg);
	}

	if (renderLine == 0)
		staticthis->vSyncPassed = true;
}

	//LOWER LIMIT: THE CODE BETWEEN THESE MARKS IS SHARED BETWEEN 3BIT, 6BIT, AND 14BIT

void IRAM_ATTR VGA3BitI::interruptPixelLine(int y, uint8_t *pixels, void *arg)
{
	VGA3BitI * staticthis = (VGA3BitI *)arg;
	unsigned long syncBits = (staticthis->hsyncBitI | staticthis->vsyncBitI) * staticthis->rendererStaticReplicate32mask;
	unsigned char *line = staticthis->frontBuffer[y];
	int j = 0;
	for (int i = 0; i < staticthis->mode.hRes / 4; i++)
	{
		int p0 = (line[j] >> 0) & 7;
		int p1 = (line[j++] >> 4) & 7;
		int p2 = (line[j] >> 0) & 7;
		int p3 = (line[j++] >> 4) & 7;
		((uint32_t *)pixels)[i] = syncBits | (p2 << 0) | (p3 << 8) | (p0 << 16) | (p1 << 24);
	}
}
