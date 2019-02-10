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
const int VGA::MODE320x400[] = {16, 96, 48, 640, 12, 2, 35, 400, 2, 1, 25175000, 1, 0};
const int VGA::MODE320x200[] = {16, 96, 48, 640, 12, 2, 35, 400, 2, 2, 25175000, 1, 0};
const int VGA::MODE320x100[] = {16, 96, 48, 640, 12, 2, 35, 400, 2, 4, 25175000, 1, 0};
const int VGA::MODE360x400[] = {16, 108, 56, 720, 11, 2, 32, 400, 2, 1, 28322000, 1, 0};
const int VGA::MODE360x200[] = {16, 108, 56, 720, 11, 2, 32, 400, 2, 2, 28322000, 1, 0};
const int VGA::MODE360x100[] = {16, 108, 56, 720, 11, 2, 32, 400, 2, 4, 28322000, 1, 0};
const int VGA::MODE360x350[] = {16, 108, 56, 720, 11, 2, 32, 350, 2, 1, 28322000, 1, 1};
const int VGA::MODE360x175[] = {16, 108, 56, 720, 11, 2, 32, 350, 2, 2, 28322000, 1, 1};

const int VGA::MODE320x350[] = {16, 96, 48, 640, 37, 2, 60, 350, 2, 1, 25175000, 0, 1};
const int VGA::MODE320x175[] = {16, 96, 48, 640, 37, 2, 60, 350, 2, 2, 25175000, 0, 1};

const int VGA::MODE400x300[] = {24, 72, 128, 800, 1, 2, 22, 600, 2, 2, 36000000, 0, 0};
const int VGA::MODE400x150[] = {24, 72, 128, 800, 1, 2, 22, 600, 2, 4, 36000000, 0, 0};
const int VGA::MODE400x100[] = {24, 72, 128, 800, 1, 2, 22, 600, 2, 6, 36000000, 0, 0};
const int VGA::MODE200x150[] = {24, 72, 128, 800, 1, 2, 22, 600, 4, 4, 36000000, 0, 0};
//const int VGA::MODE200x150[] = {40, 128, 88, 800, 1, 4, 23, 600, 4, 4, 40000000, 0, 0};	//60Hz version

//460 pixels horizontal it's based on 640x480
const int VGA::MODE460x480[] = {24, 136, 76, 920, 11, 2, 31, 480, 2, 1, 36249999, 1, 1};
const int VGA::MODE460x240[] = {24, 136, 76, 920, 11, 2, 31, 480, 2, 2, 36249999, 1, 1};
const int VGA::MODE460x120[] = {24, 136, 76, 920, 11, 2, 31, 480, 2, 4, 36249999, 1, 1};
const int VGA::MODE460x96[] = {24, 136, 76, 920, 11, 2, 31, 480, 2, 5, 36249999, 1, 1};

//base modes for custom mode calculations
const int VGA::MODE1280x1024[] = {48, 112, 248, 1280, 1, 3, 38, 1024, 1, 1, 108000000, 0, 0};
const int VGA::MODE1280x960[] = {80, 136, 216, 1280, 1, 3, 30, 960, 1, 1, 101200000, 1, 0};
const int VGA::MODE1280x800[] = {64, 136, 200, 1280, 1, 3, 24, 800, 1, 1, 83640000, 1, 0};
const int VGA::MODE1024x768[] = {24, 136, 160, 1024, 3, 6, 29, 768, 1, 1, 65000000, 1, 1};
const int VGA::MODE800x600[] = {24, 72, 128, 800, 1, 2, 22, 600, 1, 1, 36000000, 0, 0};
const int VGA::MODE720x400[] = {16, 108, 56, 720, 11, 2, 32, 400, 1, 1, 28322000, 1, 0};
const int VGA::MODE720x350[] = {16, 108, 56, 720, 11, 2, 32, 350, 2, 1, 28322000, 1, 1};
const int VGA::MODE640x480[] = {16, 96, 52, 640, 11, 2, 31, 480, 1, 1, 25175000, 1, 1};

const int VGA::bytesPerSample = 2;

VGA::VGA(const int i2sIndex)
	: I2S(i2sIndex)
{
	lineBufferCount = 8;
}

int VGA::maxXRes(const int *baseMode)
{
	return (int(baseMode[3] * 36249999. * .5 / baseMode[10]) & 0xfffffffe);
}

int *VGA::customMode(const int *baseMode, int xres, int yres, int* modeBuffer, int fixedYDivider)
{
	xres = (xres + 1) & 0xfffffffe;
	float f = float(xres) / baseMode[3];
	modeBuffer[1] = int(baseMode[1] * f + 1) & 0xfffffffe;
	modeBuffer[2] = int((baseMode[1] + baseMode[2] - modeBuffer[1] / f) * f + 1) & 0xfffffffe;
	modeBuffer[3] = xres;
	modeBuffer[0] = int(((baseMode[0] + baseMode[1] + baseMode[2] + baseMode[3]) - (modeBuffer[1] + modeBuffer[2] + modeBuffer[3]) / f) * f + 1) & 0xfffffffe;
	modeBuffer[8] = 1;
	
	modeBuffer[5] = baseMode[5];
	modeBuffer[9] = fixedYDivider ? fixedYDivider : (baseMode[7] / yres);
	modeBuffer[7] = yres * modeBuffer[9];
	modeBuffer[4] = baseMode[4] + baseMode[7] / 2 - modeBuffer[7] / 2;
	modeBuffer[6] = baseMode[6] + baseMode[7] / 2 - (modeBuffer[7] - modeBuffer[7] / 2);
	modeBuffer[10] = baseMode[10] * f;
	modeBuffer[11] = baseMode[11];
	modeBuffer[12] = baseMode[12];
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
	vSyncPassed = false;
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
	for (int i = 0; i < inactiveSamples; i++)
	{
		if (i >= hfront && i < hfront + hsync)
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
	for (int i = 0; i < hres; i++)
	{
		((unsigned short *)vSyncActiveBuffer)[i ^ 1] = hsyncBitI | vsyncBit;
		((unsigned short *)blankActiveBuffer)[i ^ 1] = hsyncBitI | vsyncBitI;
	}

	dmaBufferDescriptors = DMABufferDescriptor::allocateDescriptors(dmaBufferDescriptorCount);
	for (int i = 0; i < dmaBufferDescriptorCount; i++)
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

void VGA::vSync()
{
	vSyncPassed = true;
}

void VGA::interrupt()
{
	unsigned long *signal = (unsigned long *)dmaBufferDescriptors[dmaBufferDescriptorActive].buffer();
	unsigned long *pixels = &((unsigned long *)dmaBufferDescriptors[dmaBufferDescriptorActive].buffer())[(hsync + hback) / 2];
	unsigned long base, baseh;
	if (currentLine >= vfront && currentLine < vfront + vsync)
	{
		baseh = (vsyncBit | hsyncBit) | ((vsyncBit | hsyncBit) << 16);
		base = (vsyncBit | hsyncBitI) | ((vsyncBit | hsyncBitI) << 16);
	}
	else
	{
		baseh = (vsyncBitI | hsyncBit) | ((vsyncBitI | hsyncBit) << 16);
		base = (vsyncBitI | hsyncBitI) | ((vsyncBitI | hsyncBitI) << 16);
	}
	for (int i = 0; i < hsync / 2; i++)
		signal[i] = baseh;
	for (int i = hsync / 2; i < (hsync + hback) / 2; i++)
		signal[i] = base;

	int y = (currentLine - vfront - vsync - vback) / vdivider;
	if (y >= 0 && y < vres)
		interruptPixelLine(y, pixels, base);
	else
		for (int i = 0; i < hres / 2; i++)
		{
			pixels[i] = base | (base << 16);
		}
	for (int i = 0; i < hfront / 2; i++)
		signal[i + (hsync + hback + hres) / 2] = base;
	currentLine = (currentLine + 1) % totalLines;
	dmaBufferDescriptorActive = (dmaBufferDescriptorActive + 1) % dmaBufferDescriptorCount;
	if (currentLine == 0)
		vSync();
}

void VGA::interruptPixelLine(int y, unsigned long *pixels, unsigned long syncBits)
{
}