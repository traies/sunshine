#pragma once
#include <d3d11.h>
#include <core\Factory.h>

class AmdEncoder {
public:
	AmdEncoder(ID3D11Device * device);
	~AmdEncoder();
	bool EncodeFrame(ID3D11Texture2D * frame);
private:
	amf::AMFContextPtr _context;
	amf::AMFComponentPtr _encoder;
	bool _encoderStarted;

	bool InitContext(amf::AMFFactory ** factory);
	bool InitEncoder(ID3D11Texture2D * frame);
	bool SetEncoderProperties(int width, int height, int framerate, amf::AMF_SURFACE_FORMAT format);
};
