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
#include "ModeComposite.h"
#include "PinConfigComposite.h"

class Composite : public I2S
{
  public:
	Composite(const int i2sIndex = 0);
	bool init(const ModeComposite &mode, const int *pinMap, const int bitCount);
	virtual bool init(const ModeComposite &mode, const PinConfigComposite &pinConfig) = 0;

	static const ModeComposite MODE400x300;
	static const ModeComposite MODEPAL312P;


	static const PinConfigComposite GameWing;
	static const PinConfigComposite XPlayer;

	ModeComposite mode;

	virtual int bytesPerSample() const = 0;

  protected:	
	int lineBufferCount;
	int currentLine;
	int totalLines;	
	volatile bool vSyncPassed;
	int syncLevel;
	int blankLevel;
	int burstAmp;

	void *shortSyncBuffer;
	void *longSyncBuffer;
	void *lineSyncBuffer[2];
	void *lineBlankBuffer;
	void *lineBackBlankBuffer;

	void allocateLineBuffers(const int lines);
	virtual void allocateLineBuffers() = 0;
	virtual void allocateLineBuffers(void **frameBuffer);
	virtual void propagateResolution(const int xres, const int yres) = 0;
	virtual void getClockSetting(long *sampleRate, int *n, int *a, int *b, int *div);

  protected:
	virtual void interrupt();
	virtual void vSync();
	virtual void interruptPixelLine(int y, unsigned long *pixels);
	virtual int burst(int sampleNumber, bool even = true);
};
