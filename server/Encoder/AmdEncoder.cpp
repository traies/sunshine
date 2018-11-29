#include "AmdEncoder.h"
#include <components/VideoEncoderVCE.h>
#include <iostream>
#include <vector>

using namespace std;

variant<unique_ptr<Encoder>, sun::Error> AmdEncoder::NewEncoder(ID3D11Device * device)
{
	auto encoder = make_unique<AmdEncoder>();
	amf::AMFFactory * factory;
	auto error = encoder->InitContext(&factory);
	if (error.has_value()) {
		return error.value();
	}

	if (encoder->_context->InitDX11(device) == AMF_FAIL) {
		return error.value();
	}

	if (factory->CreateComponent(encoder->_context, AMFVideoEncoderVCE_AVC, &encoder->_encoder) == AMF_FAIL) {
		return error.value();
	}
	unique_ptr<Encoder> ret = move(encoder);

	return ret;
}

AmdEncoder::AmdEncoder()
{

}

AmdEncoder::~AmdEncoder()
{
	// TODO: Research how to release AMD encoder properly
}


// Query AMF version and init AMF context
optional<sun::Error> AmdEncoder::InitContext(amf::AMFFactory ** factory)
{
	HMODULE amfDll = LoadLibrary(AMF_DLL_NAME);
	if (amfDll == nullptr) {
		return sun::Error("Failed to load AMF");
	}

	
	auto queryVersion = reinterpret_cast<AMFQueryVersion_Fn>(GetProcAddress(amfDll, AMF_QUERY_VERSION_FUNCTION_NAME));
	amf_uint64 version;
	if (queryVersion(&version) == AMF_FAIL) {
		return sun::Error("Querying AMF version failed");
	}
	std::cout << "AMF Version: " << version << std::endl;
	
	AMFInit_Fn init = reinterpret_cast<AMFInit_Fn>(GetProcAddress(amfDll, AMF_INIT_FUNCTION_NAME));
	if (init(version, factory) == AMF_FAIL) {
		return sun::Error("AMF Init failed.");
	}
	
	if ((*factory)->CreateContext(&_context) == AMF_FAIL) {
		return sun::Error("AMF Failed to create context.");
	}
	return nullopt;
}

bool AmdEncoder::EncodeFrame(ID3D11Texture2D * frame)
{
	if (!_encoderStarted) {
		auto opt = InitEncoder(frame);
		if (opt.has_value()) {
			std::cout << opt.value().Message() << std::endl;
			return false;
		}
	}
	amf::AMFSurfacePtr wrapper;
	if (_context->CreateSurfaceFromDX11Native(frame, &wrapper, nullptr) == AMF_FAIL) {
		std::cout << "CreateSurface failed." << std::endl;
		return false;
	}
/*
	amf::AMFDataPtr dupl = nullptr;
	if (wrapper->Duplicate(wrapper->GetMemoryType(), &dupl) == AMF_FAIL) {
		std::cout << "CreateSurface failed." << std::endl;
		return false;
	}*/
	
	if (_encoder->SubmitInput(wrapper) == AMF_FAIL) {
		std::cout << "SubmintInput failed." << std::endl;
		return false;
	}

	return true;
}

void AmdEncoder::Drain()
{
	_encoder->Drain();
}

unique_ptr<FrameBuffer> AmdEncoder::PullFrame() {
	amf::AMFData * d;
	AMF_RESULT res = _encoder->QueryOutput(&d);
	if (res == AMF_OK) {
		amf::AMFBufferPtr buffer(d);
		auto ret = make_unique<AmdFrameBuffer>(buffer);
		ret->ptr = buffer->GetNative();
		ret->size = buffer->GetSize();
		return ret;
	}
	else if (res == AMF_EOF) {
		std::cout << "Reached EOF" << std::endl;
		return nullptr;
	}
	else if (res == AMF_REPEAT) {
		//std::cout << "Frame is not ready" << std::endl;
		return nullptr;
	}
	else {
		std::cout << "Unkown AMF error" << std::endl;
		return nullptr;
	}
}

optional<sun::Error> AmdEncoder::InitEncoder(ID3D11Texture2D * frame)
{
	D3D11_TEXTURE2D_DESC desc;
	frame->GetDesc(&desc);
	return SetEncoderProperties(desc.Width, desc.Height, 60, amf::AMF_SURFACE_BGRA);
}

optional<sun::Error> AmdEncoder::SetEncoderProperties(int width, int height, int framerate, amf::AMF_SURFACE_FORMAT format)
{
	_encoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR);
	_encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
	_encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, AMFConstructSize(width, height));
	_encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, AMFConstructSize(framerate, 1));
	if (_encoder->Init(format, width, height) != AMF_OK) {
		return sun::Error("Encoder init failed.");
	}
	_encoderStarted = true;
	return nullopt;
}
