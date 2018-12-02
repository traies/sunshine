#pragma once
#include <d3d11.h>
#include <memory>
#include "FrameBuffer.h"

class Encoder {
public:
	virtual ~Encoder() {}

	virtual bool EncodeFrame(ID3D11Texture2D * frame) = 0 {};
	virtual std::unique_ptr<FrameBuffer> PullFrame() = 0 {};
};