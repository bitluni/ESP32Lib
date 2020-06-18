/*
	Author: bitluni 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
		http://bitluni.net
*/
#include "ParallelLED.h"
#include "../Tools/Log.h"

const unsigned long  LEDS_PER_BUFFER = 34; //max 42

ParallelLED::ParallelLED(const int components)
	: 	I2S(1)
{
	this->components = components;
	lineBufferCount = 8;
	dmaBufferDescriptors = 0;
	gammaLut[0] = 0;
	gamma[0] = 1;
	gamma[1] = 1;
	gamma[2] = 1;
	gamma[3] = 1;
}

ParallelLED::~ParallelLED()
{
	//TODO stop i2s
	for(int i = 0; i < components; i++)
		free(gammaLut[i]);
}

bool ParallelLED::init(const int *dataPins, const unsigned long pixelPerChannel, int clockPin)
{
	//propagateResolution(xres, yres);
	for(int i = 0; i < components; i++)
		gammaLut[i] = (unsigned long *)malloc(256 * sizeof(unsigned long));
	calcGammaLUT();
	allocateBuffers(pixelPerChannel);
	//clear buffers
	for(int n = 0; n < pixelPerChannel; n++)
	{
		unsigned long desc = n / LEDS_PER_BUFFER;
		unsigned long led = n - desc * LEDS_PER_BUFFER;
		for(int i = 0; i < components * 24; i++)
		{
			((unsigned char*)buffer[0][desc])[(led * 24 * components + i) ^ 2] = ((i % 3) == 0) ? 255 : 0;
			((unsigned char*)buffer[1][desc])[(led * 24 * components + i) ^ 2] = ((i % 3) == 0) ? 255 : 0;
		}
	}
	
	
	setClock(800000, 8, false);	//not working yet
	initParallelOutputMode(dataPins, 0, 8, clockPin);
	startTX();
	return true;
}

void ParallelLED::getClockSetting(long *sampleRate, int *n, int *a, int *b, int *div)	//workaround
{
	*sampleRate = 0;
	int factor = 1;
/*	if(bitCount > 8)
		factor = 2;
	else if(bitCount > 16)
		factor = 4;*/
	*n = 40000000L / (((800000 / 8) * 6) * factor);
	*a = 1;
	*b = 0;
	*div = 1;
}

int ParallelLED::bytesPerSample() const
{
	return 1;
}

void ParallelLED::allocateBuffers(unsigned long pixelPerChannel)
{
	int bytesPerLED = 24 * components; //3 bits are needed for pulse, 8 times per component = 24
	int bytesPerBuffer = LEDS_PER_BUFFER * bytesPerLED;
	dmaBufferDescriptorCount = (pixelPerChannel + 1 + (LEDS_PER_BUFFER - 1)) / LEDS_PER_BUFFER;
	dmaBufferDescriptors = DMABufferDescriptor::allocateDescriptors(dmaBufferDescriptorCount);
	buffer[0] = (void**)malloc(sizeof(void*) * dmaBufferDescriptorCount);
	buffer[1] = (void**)malloc(sizeof(void*) * dmaBufferDescriptorCount);
	DEBUG_PRINTLN("Allocating I2S Buffers");
	DEBUG_PRINT("Buffer count ");
	DEBUG_PRINTLN(dmaBufferDescriptorCount);
	DEBUG_PRINT("Bytes per Buffer ");
	DEBUG_PRINTLN(bytesPerBuffer);
	for (int i = 0; i < dmaBufferDescriptorCount; i++)
	{
		buffer[0][i] = DMABufferDescriptor::allocateBuffer(bytesPerBuffer, true);
		buffer[1][i] = DMABufferDescriptor::allocateBuffer(bytesPerBuffer, true);
		dmaBufferDescriptors[i].setBuffer(buffer[0][i], bytesPerBuffer); 
		if (i)
			dmaBufferDescriptors[i - 1].next(dmaBufferDescriptors[i]);
	}
	dmaBufferDescriptors[dmaBufferDescriptorCount - 1].next(dmaBufferDescriptors[0]);
	currentBuffer = 1;
}

void ParallelLED::show(bool vSync)
{
	int bytesPerLED = 24 * components; //3 bits are needed for pulse, 8 times per component = 24
	int bytesPerBuffer = LEDS_PER_BUFFER * bytesPerLED;
	for (int i = 0; i < dmaBufferDescriptorCount; i++)
		dmaBufferDescriptors[i].setBuffer(buffer[currentBuffer][i], bytesPerBuffer); 
	currentBuffer = currentBuffer ^ 1;
}

bool ParallelLED::useInterrupt()
{
	return false;
}

void ParallelLED::interrupt()
{
}

void ParallelLED::setLED(const int channel, unsigned long n, unsigned long c)
{
	unsigned long desc = n / LEDS_PER_BUFFER;
	unsigned long led = n - desc * LEDS_PER_BUFFER;
	
	const int mask = 1 << channel;
	const int imask = ~mask;

	for(int j = 0; j < components; j++)
	{
		int cc = gammaLut[j][(c >> (j * 8)) & 255];
		for(int i = 0; i < 8; i++)
		{
			//((unsigned char*)buffer[currentBuffer][desc])[(led * 8 * components * 3 + j * 24 + i * 3) ^ 2] = 1;
			((unsigned char*)buffer[currentBuffer][desc])[(led * 8 * components * 3 + j * 24 + i * 3 + 1) ^ 2] = 
				(((unsigned char*)buffer[currentBuffer][desc])[(led * 8 * components * 3 + j * 24 + i * 3 + 1) ^ 2] & imask) |
				(((cc >> i) & 1) << channel);
			//((unsigned char*)buffer[currentBuffer][desc])[(led * 8 * components * 3 + j * 24 + i * 3 + 2) ^ 2] = 0;
		}
	}
}

void ParallelLED::setLED(const int channel, unsigned long n, unsigned char r, unsigned char g, unsigned char b, unsigned char w)
{
	const unsigned long c = r | (g << 8) | (b << 16) | (w << 24);
	setLED(channel, n, c);
}

void ParallelLED::setLEDGamma(int channel, unsigned long n, unsigned char r, unsigned char g, unsigned char b, unsigned char w)
{
}

void ParallelLED::setGamma(float r, float g, float b, float w)
{
	gamma[0] = r;
	gamma[1] = g;
	gamma[2] = b;
	gamma[3] = w;
	calcGammaLUT();
}

void ParallelLED::calcGammaLUT()
{
	if(!gammaLut[0]) return;
  	for(int j = 0; j < components; j++)
		for(int i = 0; i < 256; i++)
		{
			int c = (unsigned char)(pow((float)i * (1.f / 256.f), gamma[j]) * 255.f + 0.5f);
			int ic = 0;
			for(int k = 0; k < 8; k++)
				ic |= ((c >> k) & 1) << (7 - k);
			gammaLut[j][i] = ic;
		}
}
