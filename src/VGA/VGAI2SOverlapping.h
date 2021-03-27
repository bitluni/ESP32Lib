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
class VGAI2SOverlapping : public VGAI2SEngine<BufferLayout>, public GraphicsCombination
{
  public:
	typedef typename GraphicsCombination::BufferUnit BufferGraphicsUnit;
	typedef typename GraphicsCombination::Color Color;

	VGAI2SOverlapping(const int i2sIndex = 1)
		: VGAI2SEngine<BufferLayout>(i2sIndex)
	{
	}

	bool initoverlappingbuffers(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin = -1)
	{
		this->lineBufferCount = mode.vRes / mode.vDiv; // yres
		this->rendererBufferCount = this->frameBufferCount;
		return this->initengine(mode, pinMap, bitCount, clockPin, 2); // 2 buffers per line
	}

	virtual void propagateResolution(const int xres, const int yres)
	{
		this->setResolution(xres, yres);
	}

	//This auxiliary variable is a trick: when graphic tries to allocate
	//the memory for the buffers, the function returns instead the
	//pointer to the previously allocated renderer(DMA) buffers.
	//The variable helps to match each extra buffer between both
	//(renderer and canvas) systems.
	int currentBufferToAssign = 0;

	virtual BufferGraphicsUnit **allocateFrameBuffer()
	{
		void **arr = (void **)malloc(this->yres * sizeof(void *));
		if(!arr)
			ERROR("Not enough memory");
		for (int y = 0; y < this->yres; y++)
		{
			arr[y] = (void *)this->getBufferDescriptor(this->graphics_swy(y), currentBufferToAssign);
		}
		currentBufferToAssign++;
		return (BufferGraphicsUnit **)arr;
	}

	virtual void show(bool vSync = false)
	{
		if (!this->frameBufferCount)
			return;

		GraphicsCombination::show(vSync);
		this->switchToRendererBuffer(this->currentFrameBuffer);
		// wait at least one frame
		// else the switch does not take place for the display
		// until the frame is completed
		// and drawing starts in the backbuffer while still shown
		if (this->frameBufferCount == 2) // in triple buffer or single buffer this is not an issue
		{
			uint32_t timemark = micros();
			uint32_t framedurationinus = (uint64_t)this->mode.pixelsPerLine() * (uint64_t)this->mode.linesPerField() * (uint64_t)1000000 / (uint64_t)this->mode.pixelClock;
			while((micros() - timemark) < framedurationinus){delay(0);}
		}
	}

	virtual void scroll(int dy, Color color)
	{
		GraphicsCombination::scroll(dy, color);
		if(this->dmaBufferDescriptors)
			for (int i = 0; i < this->yres * this->mode.vDiv; i++)
				this->dmaBufferDescriptors[
						this->indexRendererDataBuffer[(this->currentFrameBuffer + this->frameBufferCount - 1) % this->frameBufferCount]
						 + i * this->descriptorsPerLine + this->descriptorsPerLine - 1
					].setBuffer(
							((uint8_t *) this->backBuffer[i / this->mode.vDiv]) - this->dataOffsetInLineInBytes
							,
							((this->descriptorsPerLine > 1)?this->mode.hRes:this->mode.pixelsPerLine()) * this->bytesPerBufferUnit()/this->samplesPerBufferUnit()
						);
	}
};
