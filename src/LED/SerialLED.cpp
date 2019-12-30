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
#include "SerialLED.h"

const unsigned long  LEDS_PER_BUFFER = 34;

SerialLED::SerialLED(const int i2sIndex)
	: I2S(i2sIndex)
{
	lineBufferCount = 8;
	dmaBufferDescriptors = 0;
	gamma[0] = 1;
	gamma[1] = 1;
	gamma[2] = 1;
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
	int bytesPerLED = bytesPerSample() * 3;
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

void SerialLED::setLED(unsigned long n, unsigned char r, unsigned char g, unsigned char b)
{
	unsigned long desc = n / LEDS_PER_BUFFER;
	unsigned long led = n - desc * LEDS_PER_BUFFER;
	((unsigned long*)dmaBufferDescriptors[desc].buffer())[led * 3 + 0] = lut[r];
	((unsigned long*)dmaBufferDescriptors[desc].buffer())[led * 3 + 1] = lut[g];
	((unsigned long*)dmaBufferDescriptors[desc].buffer())[led * 3 + 2] = lut[b];
}

void SerialLED::setLEDGamma(unsigned long n, unsigned char r, unsigned char g, unsigned char b)
{
	unsigned long desc = n / LEDS_PER_BUFFER;
	unsigned long led = n - desc * LEDS_PER_BUFFER;
	((unsigned long*)dmaBufferDescriptors[desc].buffer())[led * 3 + 0] = gammaLut[r][0];
	((unsigned long*)dmaBufferDescriptors[desc].buffer())[led * 3 + 1] = gammaLut[g][1];
	((unsigned long*)dmaBufferDescriptors[desc].buffer())[led * 3 + 2] = gammaLut[b][2];
}

void SerialLED::setGamma(float r, float g, float b)
{
	gamma[0] = r;
	gamma[1] = g;
	gamma[2] = b;
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
  	for(int j = 0; j < 3; j++)
		for(int i = 0; i < 256; i++)
			gammaLut[i][j] = lut[(unsigned char)(pow((float)i * (1.f / 256.f), gamma[j]) * 255.f + 0.5f)];
}