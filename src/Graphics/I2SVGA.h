#pragma once

#include "Graphics.h"
#include "../I2S/I2S.h"

class I2SVGA : public I2S, public Graphics<unsigned short>
{
  public:
	I2SVGA(const int i2sIndex);

	bool init(const int *mode, const int *redPins, const int *greenPins, const int *bluePins, const int hsyncPin, const int vsyncPin, int lineBufferCount = 8);
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

	//unusable atm
	static const int MODE400x300[];
	static const int MODE400x150[];
	static const int MODE400x100[];
	static const int MODE200x150[];

  private:
	int vsyncPin;
	int hsyncPin;
	int currentLine;
	int rgbMask;
	int vsyncBit;
	int hsyncBit;
	int vsyncBitI;
	int hsyncBitI;

	int hfront;
	int hsync;
	int hback;
	int totalLines;
	int vfront;
	int vsync;
	int vback;
	int hdivider;
	int vdivider;

  protected:
	virtual void interrupt();
	virtual void allocateDMABuffersVGA(const int lines);

  public:
	static const int HIDDEN_MODE0[];
	static const int HIDDEN_MODE1[];
	static const int HIDDEN_MODE2[];
	static const int HIDDEN_MODE3[];
};
