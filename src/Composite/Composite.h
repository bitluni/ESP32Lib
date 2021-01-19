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
#include "ModeComposite.h"
#include "PinConfigComposite.h"

#include "CompMode.h"
#include "CompositePinConfig.h"

class Composite : public I2S, public CompMode, public CompositePinConfig
{
  public:
	Composite(const int i2sIndex = 0)
		: I2S(i2sIndex)
	{
		dmaBufferDescriptors = 0;
	}

	virtual bool init(const ModeComposite &mode, const int *pinMap, const int bitCount)
	{
		this->mode = mode;
		int xres = mode.hRes;
		int yres = mode.vRes / mode.vDiv;
		blankLevel = 72;
		burstAmp = 32;
		syncLevel = 0;
		propagateResolution(xres, yres);
		totalLines = mode.linesPerField();
		allocateLineBuffers();
		currentLine = 0;
		vSyncPassed = false;
		initParallelOutputMode(pinMap, mode.pixelClock, bitCount);
		startTX();
		return true;
	}

	virtual bool init(const ModeComposite &mode, const PinConfigComposite &pinConfig) = 0;

	ModeComposite mode;

	virtual int bytesPerSample() const = 0;

  protected:	
	int lineBufferCount;
	int currentLine;
	int totalLines;	
	volatile bool vSyncPassed;
	int syncLevel;
	int blankLevel;
	int burstAmp;


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

	virtual void allocateLineBuffers() = 0;

	///complete ringbuffer from frame
	virtual void allocateLineBuffers(void **frameBuffer)
	{
		void *shortSyncBuffer;
		void *longSyncBuffer;
		void *lineSyncBuffer[2]; //this is probably wrong in class declaration as member (now moved within this function definition)
		void *lineBlankBuffer;
		void *lineBackBlankBuffer;

		//simple buffers
		dmaBufferDescriptorCount = totalLines;
		dmaBufferDescriptors = DMABufferDescriptor::allocateDescriptors(dmaBufferDescriptorCount);
		for (int i = 0; i < dmaBufferDescriptorCount; i++)
			dmaBufferDescriptors[i].next(dmaBufferDescriptors[(i + 1) % dmaBufferDescriptorCount]);
		for (int i = 0; i < totalLines; i++)
		{
			dmaBufferDescriptors[i].setBuffer(frameBuffer[i], 856);
		}
			return;

		dmaBufferDescriptorCount = totalLines * 2 + mode.vRes;
		int inactiveSamples = mode.hFront + mode.hSync;
		int activeSamples = mode.hRes;
		int syncSamples = mode.pixelsPerLine() / 2;
		
		shortSyncBuffer = DMABufferDescriptor::allocateBuffer(syncSamples * bytesPerSample(), true);
		longSyncBuffer = DMABufferDescriptor::allocateBuffer(syncSamples * bytesPerSample(), true);
		lineSyncBuffer[0] = DMABufferDescriptor::allocateBuffer(inactiveSamples * bytesPerSample(), true);
		lineSyncBuffer[1] = DMABufferDescriptor::allocateBuffer(inactiveSamples * bytesPerSample(), true);
		lineBlankBuffer = DMABufferDescriptor::allocateBuffer(mode.hRes * bytesPerSample() + mode.hBack, true, blankLevel * 0x1010101);
		lineBackBlankBuffer = DMABufferDescriptor::allocateBuffer(mode.hBack * bytesPerSample(), true, blankLevel * 0x1010101);

		if(bytesPerSample() == 1)
		{
			int i = 0;
			for (int j = 0; j < mode.hSync; j++)
			{
				((unsigned char *)(lineSyncBuffer[0]))[i ^ 2] = syncLevel;
				((unsigned char *)(lineSyncBuffer[1]))[i++ ^ 2] = syncLevel;
			}
			for (int j = 0; j < mode.hFront; j++)
			{
				((unsigned char *)(lineSyncBuffer[0]))[i ^ 2] = burst(j + mode.hSync, true);
				((unsigned char *)(lineSyncBuffer[1]))[i++ ^ 2] = burst(j + mode.hSync, false);
			}

			for (int i = 0; i < syncSamples; i++)
			{
				if(i < mode.shortSync)
					((unsigned char *)shortSyncBuffer)[i ^ 2] = blankLevel;
				else
					((unsigned char *)shortSyncBuffer)[i ^ 2] = syncLevel;
				if(i < syncSamples - mode.shortSync)
					((unsigned char *)longSyncBuffer)[i ^ 2] = blankLevel;
				else
					((unsigned char *)longSyncBuffer)[i ^ 2] = syncLevel;
			}
			for (int i = 0; i < mode.hRes + mode.hBack; i++)
				((unsigned char *)lineBlankBuffer)[i ^ 2] = blankLevel;
		}

		//bytesPerSample() == 2 is not implemented

		dmaBufferDescriptors = DMABufferDescriptor::allocateDescriptors(dmaBufferDescriptorCount);

		for (int i = 0; i < dmaBufferDescriptorCount; i++)
			dmaBufferDescriptors[i].next(dmaBufferDescriptors[(i + 1) % dmaBufferDescriptorCount]);
		int d = 0;
		for(int i = 0; i < 6; i++)
			dmaBufferDescriptors[d++].setBuffer(shortSyncBuffer, syncSamples * bytesPerSample());
		for(int i = 0; i < 5; i++)
			dmaBufferDescriptors[d++].setBuffer(longSyncBuffer, syncSamples * bytesPerSample());
		for(int i = 0; i < 5; i++)
			dmaBufferDescriptors[d++].setBuffer(shortSyncBuffer, syncSamples * bytesPerSample());

		for (int i = 0; i < mode.vFront; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(lineSyncBuffer[i & 1], inactiveSamples * bytesPerSample());
			dmaBufferDescriptors[d++].setBuffer(lineBlankBuffer, (mode.hRes + mode.hBack) * bytesPerSample());
		}

		for (int i = 0; i < mode.vRes; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(lineSyncBuffer[(i + mode.vFront) & 1], inactiveSamples * bytesPerSample());
			dmaBufferDescriptors[d++].setBuffer(frameBuffer[i / mode.vDiv], mode.hRes * bytesPerSample());
			dmaBufferDescriptors[d++].setBuffer(lineBackBlankBuffer, mode.hBack * bytesPerSample());
		}
		for (int i = 0; i < mode.vBack; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(lineSyncBuffer[(i + mode.vFront + mode.vBack) & 1], inactiveSamples * bytesPerSample());
			dmaBufferDescriptors[d++].setBuffer(lineBlankBuffer, (mode.hRes + mode.hBack) * bytesPerSample());
		}
	}

	virtual void propagateResolution(const int xres, const int yres) = 0;

	virtual void getClockSetting(long *sampleRate, int *n, int *a, int *b, int *div)
	{
		if(sampleRate)
			*sampleRate = mode.pixelClock;
		if(n)
			*n = 2;
		if(a)
			*a = 1;
		if(b)
			*b = 0;
		if(div)
			*div = 6;
	}


  protected:
	virtual void interrupt()
	{
	}

	virtual void vSync()
	{
		vSyncPassed = true;
	}

	virtual void interruptPixelLine(int y, unsigned long *pixels)
	{
	}

	virtual int burst(int sampleNumber, bool even = true)
	{
		return blankLevel;
	}
};
