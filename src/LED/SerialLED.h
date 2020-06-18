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
#pragma once

#include "../I2S/I2S.h"

class SerialLED : public I2S
{
  public:
	SerialLED(int components = 3, const int i2sIndex = 1);
	virtual ~SerialLED();
	virtual bool init(int dataPin, unsigned long pixelCount, int wordPin = -1, int clockPin = -1);
	virtual int bytesPerSample() const;
	void setLED(unsigned long n, unsigned char r, unsigned char g, unsigned char b, unsigned char w = 0);
	void setLEDGamma(unsigned long n, unsigned char r, unsigned char g, unsigned char b, unsigned char w = 0);
	void setGamma(float r, float g, float b, float w = 1.f);
	int components;
	float gamma[4];
	unsigned long *gammaLut[4];
	unsigned long lut[256];

  protected: 
	int lineBufferCount;
	void calcLUT();
	void calcGammaLUT();

	void allocateBuffers(unsigned long pixelCount);
	//virtual void propagateResolution(const int xres, const int yres) = 0;

  protected:
	virtual void interrupt();
	virtual bool useInterrupt();
};
