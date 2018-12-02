#include "stdafx.h"
#include "NvidiaEncoder.h"
#include <NvEncoder.h>


using namespace std;

NvidiaEncoder::NvidiaEncoder()
{
}


NvidiaEncoder::~NvidiaEncoder()
{
}


bool NvidiaEncoder::EncodeFrame(ID3D11Texture2D * frame) 
{
	if (_encoder == nullptr) {
		if (!InitEncoder(frame)) {
			return false;
		}
	}
	ID3D11Texture2D * inputFrame = reinterpret_cast<ID3D11Texture2D * >(_encoder->GetNextInputFrame()->inputPtr);
	ID3D11DeviceContext * context;
	ID3D11Device * device;
	frame->GetDevice(&device);
	device->GetImmediateContext(&context);
	context->CopyResource(inputFrame, frame);
	
	vector<vector<uint8_t>> buffer;
	_encoder->EncodeFrame(buffer);
	_queue.push(make_unique<NvidiaFrameBuffer>(buffer));

	return true;
}

unique_ptr<FrameBuffer> NvidiaEncoder::PullFrame()
{
	if (!_queue.empty()) {
		auto packet = move(_queue.front());
		_queue.pop();
		return packet;
	}
	else {
		return nullptr;
	}
}

bool NvidiaEncoder::InitEncoder(ID3D11Texture2D * frame)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	frame->GetDesc(&desc);
	int width = desc.Width, height = desc.Width;
	auto d3df = desc.Format;
	
	ID3D11Device * device;
	frame->GetDevice(&device);

	NV_ENC_BUFFER_FORMAT format = NV_ENC_BUFFER_FORMAT_ABGR;

	_encoder = make_unique<NvEncoderD3D11>(
		device,
		desc.Width,
		desc.Height,
		format,
		desc.Format,
		0,
		false
	);

	NV_ENC_INITIALIZE_PARAMS initParams;
	ZeroMemory(&initParams, sizeof(initParams));

	NV_ENC_CONFIG config = {NV_ENC_CONFIG_VER};
	initParams.encodeConfig = &config;
	_encoder->CreateDefaultEncoderParams(
		&initParams, 
		NV_ENC_CODEC_H264_GUID, 
		NV_ENC_PRESET_LOW_LATENCY_HP_GUID
	);
	/*config.*/
	try {
		_encoder->CreateEncoder(&initParams);
	}
	catch (exception & ex) {
		cout << ex.what() << endl;
		return false;
	}
	return true;
}


