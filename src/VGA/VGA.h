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

#include "../I2S/I2S.h"
#include "Mode.h"
#include "PinConfig.h"

// for back compatibility reasons, allow recalling "pre-configured" modes and PinConfigs within the class
// it might be deprecated in a major version increase
#include "VGAMode.h"
#include "VGAPinConfig.h"

// for back compatibility reasons, allow recalling "pre-configured" modes and PinConfigs within the class
// by inheriting VGAMode and VGAPinConfig classes, it might be deprecated in a major version increase
class VGA : public I2S, public VGAMode, public VGAPinConfig
{
  public:
	VGA(const int i2sIndex = 0)
	: I2S(i2sIndex)
	{
		lineBufferCount = 8;
		dmaBufferDescriptors = 0;
	}

	void setLineBufferCount(int lineBufferCount)
	{
		this->lineBufferCount = lineBufferCount;
	}

	virtual bool init(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin = -1)
	{
		this->mode = mode;
		int xres = mode.hRes;
		int yres = mode.vRes / mode.vDiv;
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

	virtual bool init(const Mode &mode, const PinConfig &pinConfig) = 0;

	Mode mode;

	virtual int bytesPerSample() const = 0;

  protected:
	
	virtual void initSyncBits() = 0;
	virtual long syncBits(bool h, bool v) = 0;
 
	int lineBufferCount;
	int vsyncPin;
	int hsyncPin;
	int currentLine;
	long vsyncBit;
	long hsyncBit;
	long vsyncBitI;
	long hsyncBitI;

	int totalLines;
	volatile bool vSyncPassed;

	// simple ringbuffer of blocks of size bytes each
	void allocateLineBuffers(const int lines)
	{
		dmaBufferDescriptorCount = lines;
		dmaBufferDescriptors = DMABufferDescriptor::allocateDescriptors(dmaBufferDescriptorCount);
		int bytes = (mode.hFront + mode.hSync + mode.hBack + mode.hRes) * bytesPerSample();
		for (int i = 0; i < dmaBufferDescriptorCount; i++)
		{
			dmaBufferDescriptors[i].setBuffer(DMABufferDescriptor::allocateBuffer(bytes, true), bytes); //front porch + hsync + back porch + pixels
			if (i)
				dmaBufferDescriptors[i - 1].next(dmaBufferDescriptors[i]);
		}
		dmaBufferDescriptors[dmaBufferDescriptorCount - 1].next(dmaBufferDescriptors[0]);
	}
	virtual void allocateLineBuffers()
	{
		allocateLineBuffers(lineBufferCount);
	}

	//complete ringbuffer from frame
	virtual void allocateLineBuffers(void **frameBuffer)
	{
		void *vSyncInactiveBuffer;
		void *vSyncActiveBuffer;
		void *inactiveBuffer;
		void *blankActiveBuffer;

		dmaBufferDescriptorCount = totalLines * 2;
		int inactiveSamples = (mode.hFront + mode.hSync + mode.hBack + 3) & 0xfffffffc;
		vSyncInactiveBuffer = DMABufferDescriptor::allocateBuffer(inactiveSamples * bytesPerSample(), true);
		vSyncActiveBuffer = DMABufferDescriptor::allocateBuffer(mode.hRes * bytesPerSample(), true);
		inactiveBuffer = DMABufferDescriptor::allocateBuffer(inactiveSamples * bytesPerSample(), true);
		blankActiveBuffer = DMABufferDescriptor::allocateBuffer(mode.hRes * bytesPerSample(), true);
		if(bytesPerSample() == 1)
		{
			for (int i = 0; i < inactiveSamples; i++)
			{
				if (i >= mode.hFront && i < mode.hFront + mode.hSync)
				{
					((unsigned char *)vSyncInactiveBuffer)[i ^ 2] = hsyncBit | vsyncBit;
					((unsigned char *)inactiveBuffer)[i ^ 2] = hsyncBit | vsyncBitI;
				}
				else
				{
					((unsigned char *)vSyncInactiveBuffer)[i ^ 2] = hsyncBitI | vsyncBit;
					((unsigned char *)inactiveBuffer)[i ^ 2] = hsyncBitI | vsyncBitI;
				}
			}
			for (int i = 0; i < mode.hRes; i++)
			{
				((unsigned char *)vSyncActiveBuffer)[i ^ 2] = hsyncBitI | vsyncBit;
				((unsigned char *)blankActiveBuffer)[i ^ 2] = hsyncBitI | vsyncBitI;
			}
		}
		else if(bytesPerSample() == 2)
		{
			for (int i = 0; i < inactiveSamples; i++)
			{
				if (i >= mode.hFront && i < mode.hFront + mode.hSync)
				{
					((unsigned short *)vSyncInactiveBuffer)[i ^ 1] = hsyncBit | vsyncBit;
					((unsigned short *)inactiveBuffer)[i ^ 1] = hsyncBit | vsyncBitI;
				}
				else
				{
					((unsigned short *)vSyncInactiveBuffer)[i ^ 1] = hsyncBitI | vsyncBit;
					((unsigned short *)inactiveBuffer)[i ^ 1] = hsyncBitI | vsyncBitI;
				}
			}
			for (int i = 0; i < mode.hRes; i++)
			{
				((unsigned short *)vSyncActiveBuffer)[i ^ 1] = hsyncBitI | vsyncBit;
				((unsigned short *)blankActiveBuffer)[i ^ 1] = hsyncBitI | vsyncBitI;
			}
		}

		dmaBufferDescriptors = DMABufferDescriptor::allocateDescriptors(dmaBufferDescriptorCount);
		for (int i = 0; i < dmaBufferDescriptorCount; i++)
			dmaBufferDescriptors[i].next(dmaBufferDescriptors[(i + 1) % dmaBufferDescriptorCount]);
		int d = 0;
		for (int i = 0; i < mode.vFront; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(inactiveBuffer, inactiveSamples * bytesPerSample());
			dmaBufferDescriptors[d++].setBuffer(blankActiveBuffer, mode.hRes * bytesPerSample());
		}
		for (int i = 0; i < mode.vSync; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(vSyncInactiveBuffer, inactiveSamples * bytesPerSample());
			dmaBufferDescriptors[d++].setBuffer(vSyncActiveBuffer, mode.hRes * bytesPerSample());
		}
		for (int i = 0; i < mode.vBack; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(inactiveBuffer, inactiveSamples * bytesPerSample());
			dmaBufferDescriptors[d++].setBuffer(blankActiveBuffer, mode.hRes * bytesPerSample());
		}
		for (int i = 0; i < mode.vRes; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(inactiveBuffer, inactiveSamples * bytesPerSample());
			dmaBufferDescriptors[d++].setBuffer(frameBuffer[i / mode.vDiv], mode.hRes * bytesPerSample());
		}
	}

	virtual void propagateResolution(const int xres, const int yres) = 0;

  protected:
	virtual void interrupt()
	{
		unsigned long *signal = (unsigned long *)dmaBufferDescriptors[dmaBufferDescriptorActive].buffer();
		unsigned long *pixels = &((unsigned long *)dmaBufferDescriptors[dmaBufferDescriptorActive].buffer())[(mode.hSync + mode.hBack) / 2];
		unsigned long base, baseh;
		if (currentLine >= mode.vFront && currentLine < mode.vFront + mode.vSync)
		{
			baseh = (vsyncBit | hsyncBit) | ((vsyncBit | hsyncBit) << 16);
			base = (vsyncBit | hsyncBitI) | ((vsyncBit | hsyncBitI) << 16);
		}
		else
		{
			baseh = (vsyncBitI | hsyncBit) | ((vsyncBitI | hsyncBit) << 16);
			base = (vsyncBitI | hsyncBitI) | ((vsyncBitI | hsyncBitI) << 16);
		}
		for (int i = 0; i < mode.hSync / 2; i++)
			signal[i] = baseh;
		for (int i = mode.hSync / 2; i < (mode.hSync + mode.hBack) / 2; i++)
			signal[i] = base;

		int y = (currentLine - mode.vFront - mode.vSync - mode.vBack) / mode.vDiv;
		if (y >= 0 && y < mode.vRes)
			interruptPixelLine(y, pixels, base);
		else
			for (int i = 0; i < mode.hRes / 2; i++)
			{
				pixels[i] = base | (base << 16);
			}
		for (int i = 0; i < mode.hFront / 2; i++)
			signal[i + (mode.hSync + mode.hBack + mode.hRes) / 2] = base;
		currentLine = (currentLine + 1) % totalLines;
		if (currentLine == 0)
			vSync();
	}

	virtual void vSync()
	{
		vSyncPassed = true;
	}

	virtual void interruptPixelLine(int y, unsigned long *pixels, unsigned long syncBits)
	{
	}

};
