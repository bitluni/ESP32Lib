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
#pragma once

#include "../I2S/I2S.h"

class VGA : public I2S
{
  public:
	VGA(const int i2sIndex = 0);
	void setLineBufferCount(int lineBufferCount);

	bool init(const int *mode, const int *pinMap);

	virtual float pixelAspect() const;

	static const int MODE320x480[];
	static const int MODE320x240[];
	static const int MODE320x120[];
	static const int MODE320x400[];
	static const int MODE320x200[];
	static const int MODE320x100[];
	static const int MODE360x400[];
	static const int MODE360x200[];
	static const int MODE360x100[];
	static const int MODE360x350[];
	static const int MODE360x175[];
	static const int MODE360x88[];

	//not supported on all of my screens
	static const int MODE384x576[];
	static const int MODE384x288[];
	static const int MODE384x144[];
	static const int MODE384x96[];

	static const int MODE460x480[];
	static const int MODE460x240[];
	static const int MODE460x120[];
	static const int MODE460x96[];

	//unusable atm
	static const int MODE400x300[];
	static const int MODE400x150[];
	static const int MODE400x100[];
	static const int MODE200x150[];
	
  protected:
	static const int bytesPerSample;

	int lineBufferCount;
	int vsyncPin;
	int hsyncPin;
	int currentLine;
	int vsyncBit;
	int hsyncBit;
	int vsyncBitI;
	int hsyncBitI;

	int hres;
	int hfront;
	int hsync;
	int hback;
	int totalLines;
	int vfront;
	int vsync;
	int vback;
	int hdivider;
	int vdivider;

	virtual void allocateLineBuffers(const int lines);
	virtual void setResolution(int xres, int yres) = 0;
};
