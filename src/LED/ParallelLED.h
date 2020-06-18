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

class ParallelLED : public I2S
{
  public:
  	typedef unsigned long Color;
	ParallelLED(int components = 3);
	virtual ~ParallelLED();
	virtual bool init(const int *dataPins, const unsigned long pixelPerChannel, int clockPin = -1);
	virtual int bytesPerSample() const;
	void setLED(const int channel, unsigned long n, unsigned long c);
	void setLED(const int channel, unsigned long n, unsigned char r, unsigned char g, unsigned char b, unsigned char w = 0);
	void setLEDGamma(int channel, unsigned long n, unsigned char r, unsigned char g, unsigned char b, unsigned char w = 0);
	void setGamma(float r, float g, float b, float w = 1.f);
	int components;
	float gamma[4];
	unsigned long *gammaLut[4];
	void **buffer[2];
	int currentBuffer;

	virtual void show(bool vSync = false);

  protected: 
	int lineBufferCount;
	void calcLUT();
	void calcGammaLUT();

	void allocateBuffers(unsigned long pixelPerChannel);
	//virtual void propagateResolution(const int xres, const int yres) = 0;
	virtual void getClockSetting(long *sampleRate, int *n, int *a, int *b, int *div);

  protected:
	virtual void interrupt();
	virtual bool useInterrupt();
};
