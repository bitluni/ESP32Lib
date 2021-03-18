/*
	Author: Martin-Laclaustra 2021
	License:
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out:
		https://github.com/bitluni
*/
#pragma once
#include "VGAI2SEngine.h"
#include "../Graphics/Graphics.h"

template<class BufferLayout, class GraphicsCombination>
class VGAI2SDynamic : public VGAI2SEngine<BufferLayout>, public GraphicsCombination
{
  public:
	typedef typename GraphicsCombination::BufferUnit BufferGraphicsUnit;
	typedef typename GraphicsCombination::Color Color;

	VGAI2SDynamic(const int i2sIndex = 1)
		: VGAI2SEngine<BufferLayout>(i2sIndex)
	{
	}

	bool initdynamicwritetorenderbuffer(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin = -1)
	{
		this->lineBufferCount = 3;
		this->rendererBufferCount = 1;
		return this->initengine(mode, pinMap, bitCount, clockPin, 1); // 1 buffer per line
	}

	virtual const int bitMaskInRenderingBufferHSync()
	{
		return 1<<(8*this->bytesPerBufferUnit()-2);
	}

	virtual const int bitMaskInRenderingBufferVSync()
	{
		return 1<<(8*this->bytesPerBufferUnit()-1);
	}

	virtual void initSyncBits()
	{
		this->hsyncBitI = this->mode.hSyncPolarity ? (this->bitMaskInRenderingBufferHSync()) : 0;
		this->vsyncBitI = this->mode.vSyncPolarity ? (this->bitMaskInRenderingBufferVSync()) : 0;
		this->hsyncBit = this->hsyncBitI ^ (this->bitMaskInRenderingBufferHSync());
		this->vsyncBit = this->vsyncBitI ^ (this->bitMaskInRenderingBufferVSync());
	}

	virtual long syncBits(bool hSync, bool vSync)
	{
		return ((hSync ? this->hsyncBit : this->hsyncBitI) | (vSync ? this->vsyncBit : this->vsyncBitI)) * this->rendererStaticReplicate32();
	}

	virtual void propagateResolution(const int xres, const int yres)
	{
		this->setResolution(xres, yres);
	}

	virtual void show(bool vSync = false)
	{
		if (!this->frameBufferCount)
			return;
		if (vSync)
		{
			this->vSyncPassed = false;
			while (!this->vSyncPassed)
				delay(0);
		}
		GraphicsCombination::show(vSync);
	}

  protected:
	bool useInterrupt()
	{
		return true;
	};
};
