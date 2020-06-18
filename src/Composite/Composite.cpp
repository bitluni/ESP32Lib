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

const ModeComposite Composite::MODE400x300(32, 76, 380, 8, 8, 24, 278, 2, 1, 16, 8000000);

//1136 per line 
//920 visible
//84 sync
//132 front
//312 total lines

//856
const ModeComposite Composite::MODEPAL312P(64, 96, 640, 56, 8, 23, 272, 9, 1, 32, 13333333, 70, 38, 4433619);
//const ModeComposite Composite::MODEPAL312P(84, 152, 840, 60, 8, 23, 272, 9, 1, 42, 17734475, 99, 40, 4433619);
//const ModeComposite   Composite::MODEPAL312P(44, 76, 420, 28, 8, 23, 272, 9, 1, 20, 8867238, 50, 20, 4433619);
//const ModeComposite Composite::MODENTSC312P(64, 96, 640, 56, 8, 23, 272, 9, 1, 32, 13333333, 70, 38, 4433619);

//4.43361875 * 4
const PinConfigComposite Composite::GameWing(22, 14, 32, 15, 33, 27, 12, 13);
const PinConfigComposite Composite::XPlayer(32, 27, 14, 12, 13, 4, 21, 22);

Composite::Composite(const int i2sIndex)
	: I2S(i2sIndex)
{
	dmaBufferDescriptors = 0;
}

bool Composite::init(const ModeComposite &mode, const int *pinMap, const int bitCount)
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

void Composite::getClockSetting(long *sampleRate, int *n, int *a, int *b, int *div)
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

int Composite::burst(int sampleNumber, bool even)
{
	return blankLevel;
}

///complete ringbuffer from frame
void Composite::allocateLineBuffers(void **frameBuffer)
{
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