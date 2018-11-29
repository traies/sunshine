#pragma once
#include <d3d11.h>
#include <core\Factory.h>
#include "../common/Error/Error.h"
#include <tuple>
#include <variant>
#include <optional>
#include "FrameBuffer.h"
class AmdFrameBuffer : public FrameBuffer {
public:
	AmdFrameBuffer(amf::AMFBufferPtr data): data(data) {}

	~AmdFrameBuffer() {
		data->Release();
	}

private:
	amf::AMFDataPtr data;
};

class AmdEncoder {
public:
	static std::variant<AmdEncoder, sun::Error> NewEncoder(ID3D11Device * device);
	~AmdEncoder();
	bool EncodeFrame(ID3D11Texture2D * frame);
	std::unique_ptr<FrameBuffer> PullFrame();
	void Drain();
private:
	amf::AMFContextPtr _context;
	amf::AMFComponentPtr _encoder;
	bool _encoderStarted;
	
	AmdEncoder();
	std::optional<sun::Error> InitContext(amf::AMFFactory ** factory);
	std::optional<sun::Error> InitEncoder(ID3D11Texture2D * frame);
	std::optional<sun::Error> SetEncoderProperties(int width, int height, int framerate, amf::AMF_SURFACE_FORMAT format);
};
