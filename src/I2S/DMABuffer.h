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

class DMABuffer
{
  public:
	lldesc_t descriptor;
	unsigned char *buffer;

	static DMABuffer *allocate(int bytes, bool clear = true)
	{
		DMABuffer *b = (DMABuffer *)heap_caps_malloc(sizeof(DMABuffer), MALLOC_CAP_DMA);
		if (!b)
			DEBUG_PRINTLN("Failed to alloc DMABuffer class");
		b->init(bytes, clear);
		return b;
	}

	bool init(unsigned char *buffer, int bytes, bool clear = true)
	{
		if (!buffer)
			return false;
		this->buffer = buffer;
		if (clear)
			for (int i = 0; i < bytes; i++)
				buffer[i] = 0;
		descriptor.length = bytes;
		descriptor.size = descriptor.length;
		descriptor.owner = 1;
		descriptor.sosf = 1;
		descriptor.buf = (uint8_t *)buffer;
		descriptor.offset = 0;
		descriptor.empty = 0;
		descriptor.eof = 1;
		descriptor.qe.stqe_next = 0;
		return true;
	}

	bool init(int bytes, bool clear = true)
	{
		return init((unsigned char *)heap_caps_malloc(bytes, MALLOC_CAP_DMA), bytes, clear);
	}

	void next(DMABuffer *next)
	{
		descriptor.qe.stqe_next = &(next->descriptor);
	}

	int sampleCount() const
	{
		return descriptor.length / 4;
	}

	void destroy()
	{
		if (buffer)
		{
			free(buffer);
			buffer = 0;
		}
		free(this);
	}
};
