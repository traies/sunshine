#pragma once
#include <d3d11.h>
#include <core\Factory.h>
#include "../common/Error/Error.h"
#include <tuple>
#include <variant>

class AmdEncoder {
public:
	static std::variant<AmdEncoder, sun::Error> NewEncoder(ID3D11Device * device);
	~AmdEncoder();
	bool EncodeFrame(ID3D11Texture2D * frame);
	

private:
	amf::AMFContextPtr _context;
	amf::AMFComponentPtr _encoder;

	AmdEncoder();
	bool InitContext(amf::AMFFactory ** factory);
	bool InitEncoder(ID3D11Texture2D * frame);
	bool _encoderStarted;
	bool SetEncoderProperties(int width, int height, int framerate, amf::AMF_SURFACE_FORMAT format);
};
