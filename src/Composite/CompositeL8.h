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
#include "Composite.h"
#include "../Graphics/GraphicsL8CompositeSwapped.h"

class CompositeL8 : public Composite, public GraphicsL8CompositeSwapped
{
  public:
	CompositeL8() //8 bit based modes only work with I2S1
		: Composite(1)
	{
	}


	bool init(const ModeComposite &mode, 
			  const int C0Pin, const int C1Pin,
			  const int C2Pin, const int C3Pin,
			  const int C4Pin, const int C5Pin,
			  const int C6Pin, const int C7Pin)
	{
		int pinMap[8] = {
			C0Pin, C1Pin,
			C2Pin, C3Pin,
			C4Pin, C5Pin,
			C6Pin, C7Pin
		};

		return Composite::init(mode, pinMap, 8);
	}

	bool init(const ModeComposite &mode, const int *compositePins)
	{
		return Composite::init(mode, compositePins, 8);
	}

	bool init(const ModeComposite &mode, const PinConfigComposite &pinConfig)
	{
		int pins[8];
		pinConfig.fill(pins);
		return Composite::init(mode, pins, 8);
	}

	virtual int bytesPerSample() const
	{
		return 1;
	}

	virtual float pixelAspect() const
	{
		return 1;
	}

	virtual void propagateResolution(const int xres, const int yres)
	{
		setResolution(xres, yres);
	}

	void *vSyncInactiveBuffer;
	void *vSyncActiveBuffer;
	void *inactiveBuffer;
	void *blankActiveBuffer;

	virtual Color **allocateFrameBuffer()
	{
		return (Color **)DMABufferDescriptor::allocateDMABufferArray(yres, mode.hRes * bytesPerSample(), true, 75 * 0x1010101);
	}

	virtual void allocateLineBuffers()
	{
		Composite::allocateLineBuffers((void **)frameBuffers[0]);
	}

	virtual void show(bool vSync = false)
	{
		if (!frameBufferCount)
			return;
		if (vSync)
		{
			//TODO read the I2S docs to find out
		}
		Graphics::show(vSync);
		if(dmaBufferDescriptors)
		for (int i = 0; i < yres * mode.vDiv; i++)
			dmaBufferDescriptors[(mode.vFront + mode.vSync + mode.vBack + i) * 2 + 1].setBuffer(frontBuffer[i / mode.vDiv], mode.hRes * bytesPerSample());
	}

	virtual void scroll(int dy, Color color)
	{
		Graphics::scroll(dy, color);
		if (frameBufferCount == 1)
			show();
	}

  protected:
	virtual void interrupt()
	{
		Serial.print('.');
	}
};
