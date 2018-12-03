#pragma once
#include "Encoder\Encoder.h"
#include <NvEncoder\NvEncoderD3D11.h>
#include <queue>

class NvidiaFrameBuffer : public FrameBuffer {
public:
	NvidiaFrameBuffer(std::vector<std::vector<uint8_t>> buffer): buffer(buffer) {
		auto buf = buffer.data()->data();
		ptr = reinterpret_cast<void *>(buf);
		size = buffer.data()->size();
	}
private:
	std::vector<std::vector<uint8_t>> buffer;
};

class NvidiaEncoder : public Encoder
{
public:
	NvidiaEncoder();
	~NvidiaEncoder();
	
	bool EncodeFrame(ID3D11Texture2D * texture);
	std::unique_ptr<FrameBuffer> PullFrame();

private:
	std::unique_ptr<NvEncoderD3D11> _encoder;
	std::queue<std::unique_ptr<FrameBuffer>> _queue;
	
	bool InitEncoder(ID3D11Texture2D * frame);

};

