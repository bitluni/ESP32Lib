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

class SerialLED : public I2S
{
  public:
	SerialLED(const int i2sIndex = 1);
	virtual bool init(int dataPin, unsigned long pixelCount, int wordPin = -1, int clockPin = -1);
	virtual int bytesPerSample() const;
	void setLED(unsigned long n, unsigned char r, unsigned char g, unsigned char b);
	void setLEDGamma(unsigned long n, unsigned char r, unsigned char g, unsigned char b);
	void setGamma(float r, float g, float b);
	float gamma[3];
	unsigned long gammaLut[256][3];
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
