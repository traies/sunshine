#pragma once
#include <stdint.h>

class FrameBuffer
{
public:
	virtual ~FrameBuffer() {}

	void * ptr;
	int size;
};

