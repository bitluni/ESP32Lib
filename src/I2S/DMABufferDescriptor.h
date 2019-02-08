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
#include "../Tools/Log.h"

class DMABufferDescriptor : public lldesc_t
{
  public:
	static void *allocateBuffer(int bytes, bool clear = true)
	{
		bytes = (bytes + 3) & 0xfffffffc;
		void *b = (unsigned long *)heap_caps_malloc(bytes, MALLOC_CAP_DMA);
		if (!b)
			DEBUG_PRINTLN("Failed to alloc dma buffer");
		else
			if (clear)
				for (int i = 0; i < bytes / 4; i++)
					((unsigned long*)b)[i] = 0;
		return b;
	}

	void setBuffer(void *buffer)
	{
		buf = (uint8_t *)buffer;
	}

	unsigned long *buffer() const
	{
		return (unsigned long *)buf;
	}

	void init(int bytes)
	{
		length = bytes;
		size = length;
		owner = 1;
		sosf = 1;
		buf = (uint8_t *)0;
		offset = 0;
		empty = 0;
		eof = 1;
		qe.stqe_next = 0;
	}

	static DMABufferDescriptor *allocateDescriptor(int bytes, bool allocBuffer = true, bool clear = true)
	{
		bytes = (bytes + 3) & 0xfffffffc;
		DMABufferDescriptor *b = (DMABufferDescriptor *)heap_caps_malloc(sizeof(DMABufferDescriptor), MALLOC_CAP_DMA);
		if (!b)
			DEBUG_PRINTLN("Failed to alloc DMABufferDescriptor");
		b->init(bytes);
		if(allocateBuffer)
			b->setBuffer(allocateBuffer(bytes, clear));
		return b;
	}

	void next(DMABufferDescriptor *next)
	{
		qe.stqe_next = next;
	}

	int sampleCount() const
	{
		return length / 4;
	}

	void destroy()
	{
		if (buf)
		{
			free((void*)buf);
			buf = 0;
		}
		free(this);
	}
};