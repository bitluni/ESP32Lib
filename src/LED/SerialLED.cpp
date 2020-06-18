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
#include "SerialLED.h"

const unsigned long  LEDS_PER_BUFFER = 34;

SerialLED::SerialLED(const int components, const int i2sIndex)
	: I2S(i2sIndex)
{
	this->components = components;
	lineBufferCount = 8;
	dmaBufferDescriptors = 0;
	gamma[0] = 1;
	gamma[1] = 1;
	gamma[2] = 1;
	gamma[3] = 1;
	for(int i = 0; i < components; i++)
		gammaLut[i] = (unsigned long *)malloc(256 * sizeof(unsigned long));
}

SerialLED::~SerialLED()
{
	//TODO stop i2s
	for(int i = 0; i < components; i++)
		free(gammaLut[i]);
}

bool SerialLED::init(int dataPin, unsigned long pixelCount, int wordPin, int clockPin)
{
	//propagateResolution(xres, yres);
	calcLUT();
	calcGammaLUT();
	allocateBuffers(pixelCount);
	setClock((800000 / 8) * 3, 24, false);
	initSerialOutputMode(dataPin, 24, wordPin, clockPin);
	startTX();
	return true;
}

int SerialLED::bytesPerSample() const
{
	return 4;
}

void SerialLED::allocateBuffers(unsigned long pixelCount)
{
	int bytesPerLED = bytesPerSample() * components;
	int bytesPerBuffer = LEDS_PER_BUFFER * bytesPerLED;
	dmaBufferDescriptorCount = (pixelCount + 1 + (LEDS_PER_BUFFER - 1)) / LEDS_PER_BUFFER;
	dmaBufferDescriptors = DMABufferDescriptor::allocateDescriptors(dmaBufferDescriptorCount);
	for (int i = 0; i < dmaBufferDescriptorCount; i++)
	{
		dmaBufferDescriptors[i].setBuffer(DMABufferDescriptor::allocateBuffer(bytesPerBuffer, true), bytesPerBuffer); //front porch + hsync + back porch + pixels
		if (i)
			dmaBufferDescriptors[i - 1].next(dmaBufferDescriptors[i]);
	}
	dmaBufferDescriptors[dmaBufferDescriptorCount - 1].next(dmaBufferDescriptors[0]);
}

bool SerialLED::useInterrupt()
{
	return false;
}

void SerialLED::interrupt()
{
}

void SerialLED::setLED(unsigned long n, unsigned char r, unsigned char g, unsigned char b, unsigned char w)
{
	const int c[] = {r, g, b, w};
	unsigned long desc = n / LEDS_PER_BUFFER;
	unsigned long led = n - desc * LEDS_PER_BUFFER;
	for(int i = 0;  i < components; i++)
		((unsigned long*)dmaBufferDescriptors[desc].buffer())[led * components + i] = lut[c[i]];
}

void SerialLED::setLEDGamma(unsigned long n, unsigned char r, unsigned char g, unsigned char b, unsigned char w)
{
	const int c[] = {r, g, b, w};
	unsigned long desc = n / LEDS_PER_BUFFER;
	unsigned long led = n - desc * LEDS_PER_BUFFER;
	for(int i = 0;  i < components; i++)
		((unsigned long*)dmaBufferDescriptors[desc].buffer())[led * components + i] = gammaLut[i][c[i]];
}

void SerialLED::setGamma(float r, float g, float b, float w)
{
	gamma[0] = r;
	gamma[1] = g;
	gamma[2] = b;
	gamma[3] = w;
	calcGammaLUT();
}

void SerialLED::calcLUT()
{
	for(int v = 0; v < 256; v++)
	{
		unsigned long b = 0;
		for(int bit = 7; bit >= 0; bit--)
			b |= ((((v >> bit) & 1) << 1) | 0b100) << (bit * 3 + 8);
		lut[v] = b;
	}
}

void SerialLED::calcGammaLUT()
{
  	for(int j = 0; j < components; j++)
		for(int i = 0; i < 256; i++)
			gammaLut[j][i] = lut[(unsigned char)(pow((float)i * (1.f / 256.f), gamma[j]) * 255.f + 0.5f)];
}