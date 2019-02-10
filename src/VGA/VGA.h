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

	int maxXRes(const int *baseMode);
	int *customMode(const int *baseMode, int xres, int yres, int* modeBuffer, int fiyedYDivider = 0); 
	int *customWideMode(int xres, int yres, int* modeBuffer, int fiyedYDivider = 0); 
	bool init(const int *mode, const int *pinMap);

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

	static const int MODE320x350[];
	static const int MODE320x175[];

	static const int MODE460x480[];
	static const int MODE460x240[];
	static const int MODE460x120[];
	static const int MODE460x96[];

	//unusable atm
	static const int MODE400x300[];
	static const int MODE400x150[];
	static const int MODE400x100[];
	//works
	static const int MODE200x150[];

	static const int MODE1280x1024[];
	static const int MODE1280x960[];
	static const int MODE1280x800[];
	static const int MODE1024x768[];
	static const int MODE800x600[];
	static const int MODE720x400[];
	static const int MODE720x350[];
	static const int MODE640x480[];

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
	int vres;
	int hfront;
	int hsync;
	int hback;
	int totalLines;
	int vfront;
	int vsync;
	int vback;
	int hdivider;
	int vdivider;
	volatile bool vSyncPassed;

	void *vSyncInactiveBuffer;
	void *vSyncActiveBuffer;
	void *inactiveBuffer;
	void *blankActiveBuffer;

	void allocateLineBuffers(const int lines);
	virtual void allocateLineBuffers();
	virtual void allocateLineBuffers(void **frameBuffer);
	virtual void propagateResolution(const int xres, const int yres) = 0;

  protected:
	virtual void interrupt();
	virtual void vSync();
	virtual void interruptPixelLine(int y, unsigned long *pixels, unsigned long syncBits);
};
