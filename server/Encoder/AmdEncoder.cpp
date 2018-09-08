#include "AmdEncoder.h"
#include <components/VideoEncoderVCE.h>
#include <iostream>
#include <vector>

AmdEncoder::AmdEncoder(ID3D11Device * device)
{
	amf::AMFFactory * factory;
	if (!InitContext(&factory)) {
		throw std::runtime_error("Could not initialize context.");
	}

	if (_context->InitDX11(device) == AMF_FAIL) {
		throw std::runtime_error("Could not InitDX11.");
	}

	if (factory->CreateComponent(_context, AMFVideoEncoderVCE_AVC, &_encoder) == AMF_FAIL) {
		throw std::runtime_error("Could not create AMD encoder.");
	}

}

AmdEncoder::~AmdEncoder()
{
	// TODO: Research how to release AMD encoder properly
}


// Query AMF version and init AMF context
bool AmdEncoder::InitContext(amf::AMFFactory ** factory)
{
	HMODULE amfDll = LoadLibrary(AMF_DLL_NAME);
	if (amfDll == nullptr) {
		std::cout << "Failed to load AMF" << std::endl;
		return false;
	}

	
	auto queryVersion = reinterpret_cast<AMFQueryVersion_Fn>(GetProcAddress(amfDll, AMF_QUERY_VERSION_FUNCTION_NAME));
	amf_uint64 version;
	if (queryVersion(&version) == AMF_FAIL) {
		std::cout << "Querying AMF version failed" << std::endl;
		return false;
	}
	std::cout << "AMF Version: " << version << std::endl;
	
	AMFInit_Fn init = reinterpret_cast<AMFInit_Fn>(GetProcAddress(amfDll, AMF_INIT_FUNCTION_NAME));
	if (init(version, factory) == AMF_FAIL) {
		std::cout << "AMF Init failed." << std::endl;
		return false;
	}
	
	if ((*factory)->CreateContext(&_context) == AMF_FAIL) {
		std::cout << "AMF Failed to create context." << std::endl;
		return false;
	}
}

bool AmdEncoder::EncodeFrame(ID3D11Texture2D * frame)
{
	if (!_encoderStarted) {
		if (!InitEncoder(frame)) {
			throw std::runtime_error("Encoder Init failed.");
		}
	}
	amf::AMFSurfacePtr wrapper;
	if (_context->CreateSurfaceFromDX11Native(frame, &wrapper, nullptr) == AMF_FAIL) {
		std::cout << "CreateSurface failed." << std::endl;
		return false;
	}
	
	if (_encoder->SubmitInput(wrapper) == AMF_FAIL) {
		std::cout << "SubmintInput failed." << std::endl;
		return false;
	}

	return true;
}

bool AmdEncoder::InitEncoder(ID3D11Texture2D * frame)
{
	D3D11_TEXTURE2D_DESC desc;
	frame->GetDesc(&desc);
	return SetEncoderProperties(desc.Width, desc.Height, 60, amf::AMF_SURFACE_RGBA);
}

bool AmdEncoder::SetEncoderProperties(int width, int height, int framerate, amf::AMF_SURFACE_FORMAT format)
{
	_encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
	_encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, AMFConstructSize(width, height));
	_encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, AMFConstructSize(framerate, 1));
	return _encoder->Init(format, width, height) == AMF_OK;
}
