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
#include "VGA.h"

//maximum pixel clock with apll is 36249999.
//hfront hsync hback pixels vfront vsync vback lines divx divy pixelclock hpolaritynegative vpolaritynegative
const int VGA::MODE320x480[] = {16, 96, 52, 640, 11, 2, 31, 480, 2, 1, 25175000, 1, 1};
const int VGA::MODE320x240[] = {16, 96, 52, 640, 11, 2, 31, 480, 2, 2, 25175000, 1, 1};
const int VGA::MODE320x120[] = {16, 96, 52, 640, 11, 2, 31, 480, 2, 4, 25175000, 1, 1};
const int VGA::MODE320x400[] = {16, 96, 48, 640, 11, 2, 31, 400, 2, 1, 25175000, 1, 0};
const int VGA::MODE320x200[] = {16, 96, 48, 640, 11, 2, 31, 400, 2, 2, 25175000, 1, 0};
const int VGA::MODE320x100[] = {16, 96, 48, 640, 11, 2, 31, 400, 2, 4, 25175000, 1, 0};
const int VGA::MODE360x400[] = {16, 108, 56, 720, 11, 2, 32, 400, 2, 1, 28322000, 1, 0};
const int VGA::MODE360x200[] = {16, 108, 56, 720, 11, 2, 32, 400, 2, 2, 28322000, 1, 0};
const int VGA::MODE360x100[] = {16, 108, 56, 720, 11, 2, 32, 400, 2, 4, 28322000, 1, 0};
const int VGA::MODE360x350[] = {16, 108, 56, 720, 11, 2, 32, 350, 2, 1, 28322000, 1, 1};
const int VGA::MODE360x175[] = {16, 108, 56, 720, 11, 2, 32, 350, 2, 2, 28322000, 1, 1};
const int VGA::MODE360x88[] = {16, 108, 56, 720, 11, 2, 31, 350, 2, 4, 28322000, 1, 1};

const int VGA::MODE320x350[] = {16, 96, 48, 640, 37, 2, 60, 350, 2, 1, 25175000, 1, 1};
const int VGA::MODE320x175[] = {16, 96, 48, 640, 37, 2, 60, 350, 2, 2, 25175000, 1, 1};

//not supported on any of my screens
const int VGA::MODE384x576[] = {24, 80, 104, 768, 1, 3, 17, 576, 2, 1, 34960000, 1, 0};
const int VGA::MODE384x288[] = {24, 80, 104, 768, 1, 3, 17, 576, 2, 2, 34960000, 1, 0};
const int VGA::MODE384x144[] = {24, 80, 104, 768, 1, 3, 17, 576, 2, 4, 34960000, 1, 0};
const int VGA::MODE384x96[] = {24, 80, 104, 768, 1, 3, 17, 576, 2, 6, 34960000, 1, 0};

//not stable (can't reach 40MHz pixel clock, it's clipped by the driver to 36249999 at undivided resolution)
//you can mod the timings a bit the get it running on your system
const int VGA::MODE400x300[] = {40, 128, 88, 800, 1, 4, 23, 600, 2, 2, 39700000, 0, 0};
const int VGA::MODE400x150[] = {40, 128, 88, 800, 1, 4, 23, 600, 2, 4, 39700000, 0, 0};
const int VGA::MODE400x100[] = {40, 128, 88, 800, 1, 4, 23, 600, 2, 6, 39700000, 0, 0};
//works
const int VGA::MODE200x150[] = {40, 128, 88, 800, 1, 4, 23, 600, 4, 4, 39700000, 0, 0};

//460 pixels horizontal it's based on 640x480
const int VGA::MODE460x480[] = {24, 136, 76, 920, 11, 2, 31, 480, 2, 1, 36249999, 1, 1};
const int VGA::MODE460x240[] = {24, 136, 76, 920, 11, 2, 31, 480, 2, 2, 36249999, 1, 1};
const int VGA::MODE460x120[] = {24, 136, 76, 920, 11, 2, 31, 480, 2, 4, 36249999, 1, 1};
const int VGA::MODE460x96[] = {24, 136, 76, 920, 11, 2, 31, 480, 2, 5, 36249999, 1, 1};

const int VGA::bytesPerSample = 2;

VGA::VGA(const int i2sIndex)
	: I2S(i2sIndex)
{
	lineBufferCount = 8;
}

bool VGA::init(const int *mode, const int *pinMap)
{
	int xres = mode[3] / mode[8];
	int yres = mode[7] / mode[9];
	hres = xres;
	vres = yres;
	hsyncBitI = mode[11] << 14;
	vsyncBitI = mode[12] << 15;
	hsyncBit = hsyncBitI ^ 0x4000;
	vsyncBit = vsyncBitI ^ 0x8000;
	propagateResolution(xres, yres);
	this->vsyncPin = vsyncPin;
	this->hsyncPin = hsyncPin;
	hdivider = mode[8];
	vdivider = mode[9];
	hfront = mode[0] / hdivider;
	hsync = mode[1] / hdivider;
	hback = mode[2] / hdivider;
	totalLines = mode[4] + mode[5] + mode[6] + mode[7];
	vfront = mode[4];
	vsync = mode[5];
	vback = mode[6];
	initParallelOutputMode(pinMap, mode[10] / hdivider);
	allocateLineBuffers();
	currentLine = 0;
	startTX();
	return true;
}

void VGA::setLineBufferCount(int lineBufferCount)
{
	this->lineBufferCount = lineBufferCount;
}

void VGA::allocateLineBuffers()
{
	allocateLineBuffers(lineBufferCount);
}

/// simple ringbuffer of blocks of size bytes each
void VGA::allocateLineBuffers(const int lines)
{
	dmaBufferDescriptorCount = lines;
	dmaBufferDescriptors = DMABufferDescriptor::allocateDescriptors(dmaBufferDescriptorCount);
	int bytes = (hfront + hsync + hback + hres) * bytesPerSample;
	for (int i = 0; i < dmaBufferDescriptorCount; i++)
	{
		dmaBufferDescriptors[i].setBuffer(DMABufferDescriptor::allocateBuffer(bytes, true), bytes); //front porch + hsync + back porch + pixels
		if (i)
			dmaBufferDescriptors[i - 1].next(dmaBufferDescriptors[i]);
	}
	dmaBufferDescriptors[dmaBufferDescriptorCount - 1].next(dmaBufferDescriptors[0]);
}

///complete rinbuffer from frame
void VGA::allocateLineBuffers(void **frameBuffer)
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
	for (int i = 0; i < vres * vdivider; i++)
	{
		dmaBufferDescriptors[d++].setBuffer(inactiveBuffer, inactiveSamples * bytesPerSample);
		dmaBufferDescriptors[d++].setBuffer(frameBuffer[i / vdivider], hres * bytesPerSample);
	}
}
