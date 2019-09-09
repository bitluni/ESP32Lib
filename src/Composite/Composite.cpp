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
#include "Composite.h"

const ModeComposite Composite::MODE400x300(0, 32, 76, 388, 8, 24, 278, 2, 1, 16, 8000000);
//4.43361875 * 4
const PinConfigComposite Composite::GameWing(22, 14, 32, 15, 33, 27, 12, 13);
const PinConfigComposite Composite::XPlayer(32, 27, 14, 12, 13, 4, 21, 22);

Composite::Composite(const int i2sIndex)
	: I2S(i2sIndex)
{
	lineBufferCount = 8;
	dmaBufferDescriptors = 0;
}

bool Composite::init(const ModeComposite &mode, const int *pinMap, const int bitCount)
{
	this->mode = mode;
	int xres = mode.hRes;
	int yres = mode.vRes / mode.vDiv;
	propagateResolution(xres, yres);
	totalLines = mode.linesPerField();
	allocateLineBuffers();
	currentLine = 0;
	vSyncPassed = false;
	initParallelOutputMode(pinMap, mode.pixelClock, bitCount);
	startTX();
	return true;
}

void Composite::setLineBufferCount(int lineBufferCount)
{
	this->lineBufferCount = lineBufferCount;
}

void Composite::allocateLineBuffers()
{
	allocateLineBuffers(lineBufferCount);
}

/// simple ringbuffer of blocks of size bytes each
void Composite::allocateLineBuffers(const int lines)
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

///complete ringbuffer from frame
void Composite::allocateLineBuffers(void **frameBuffer)
{
	dmaBufferDescriptorCount = totalLines * 2;
	int inactiveSamples = mode.hFront + mode.hSync + mode.hBack;
	int activeSamples = mode.hRes;
	int syncSamples = mode.pixelsPerLine() / 2;
	Serial.println(syncSamples);
	const int blankLevel = 75;
	const int syncLevel = 0;
	
	shortSyncBuffer = DMABufferDescriptor::allocateBuffer(syncSamples * bytesPerSample(), true);
	longSyncBuffer = DMABufferDescriptor::allocateBuffer(syncSamples * bytesPerSample(), true);
	lineSyncBuffer = DMABufferDescriptor::allocateBuffer(inactiveSamples * bytesPerSample(), true);
	lineBlankBuffer = DMABufferDescriptor::allocateBuffer(mode.hRes * bytesPerSample(), true, blankLevel);

	if(bytesPerSample() == 1)
	{
		int i = 0;
		for (int j = 0; j < mode.hBack; j++)
			((unsigned char *)lineSyncBuffer)[i++ ^ 2] = blankLevel;
		for (int j = 0; j < mode.hSync; j++)
			((unsigned char *)lineSyncBuffer)[i++ ^ 2] = syncLevel;
		for (int j = 0; j < mode.hFront; j++)
			((unsigned char *)lineSyncBuffer)[i++ ^ 2] = blankLevel;
		
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
		for (int i = 0; i < mode.hRes; i++)
			((unsigned char *)lineBlankBuffer)[i ^ 2] = blankLevel;
	}
	else if(bytesPerSample() == 2)
	{
		int i = 0;
		for (int j = 0; j < mode.hBack; j++)
			((unsigned short *)lineSyncBuffer)[i++ ^ 1] = blankLevel;
		for (int j = 0; j < mode.hSync; j++)
			((unsigned short *)lineSyncBuffer)[i++ ^ 1] = syncLevel;
		for (int j = 0; j < mode.hFront; j++)
			((unsigned short *)lineSyncBuffer)[i++ ^ 1] = blankLevel;
		
		for (int i = 0; i < syncSamples; i++)
		{
			if(i < mode.shortSync)
				((unsigned short *)shortSyncBuffer)[i++ ^ 1] = blankLevel;
			else
				((unsigned short *)shortSyncBuffer)[i++ ^ 1] = syncLevel;
			if(i < syncSamples - mode.shortSync)
				((unsigned short *)longSyncBuffer)[i++ ^ 1] = blankLevel;
			else
				((unsigned short *)longSyncBuffer)[i++ ^ 1] = syncLevel;
		}
		for (int i = 0; i < mode.hRes; i++)
			((unsigned short *)lineBlankBuffer)[i ^ 1] = blankLevel;
	}

	dmaBufferDescriptors = DMABufferDescriptor::allocateDescriptors(dmaBufferDescriptorCount);
//dmaBufferDescriptorCount = 2;
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
		dmaBufferDescriptors[d++].setBuffer(lineSyncBuffer, inactiveSamples * bytesPerSample());
		dmaBufferDescriptors[d++].setBuffer(lineBlankBuffer, mode.hRes * bytesPerSample());
	}
	for (int i = 0; i < mode.vRes; i++)
	{
		dmaBufferDescriptors[d++].setBuffer(lineSyncBuffer, inactiveSamples * bytesPerSample());
		dmaBufferDescriptors[d++].setBuffer(frameBuffer[i / mode.vDiv], mode.hRes * bytesPerSample());
	}
	for (int i = 0; i < mode.vBack; i++)
	{
		dmaBufferDescriptors[d++].setBuffer(lineSyncBuffer, inactiveSamples * bytesPerSample());
		dmaBufferDescriptors[d++].setBuffer(lineBlankBuffer, mode.hRes * bytesPerSample());
	}
}

void Composite::vSync()
{
	vSyncPassed = true;
}

void Composite::interrupt()
{
}

void Composite::interruptPixelLine(int y, unsigned long *pixels)
{
}