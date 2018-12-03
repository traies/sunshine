#include "stdafx.h"
#include "AvEncoder.h"
#include <iostream>
#include <d3d11.h>
#define RNDTO2(X) ( ( (X) & 0xFFFFFFFE )
#define RNDTO32(X) ( ( (X) % 32 ) ? ( ( (X) + 32 ) & 0xFFFFFFE0 ) : (X) )
using namespace std;

AvEncoder::AvEncoder()
{
}


AvEncoder::~AvEncoder()
{
}

bool AvEncoder::InitEncoder(ID3D11Texture2D * frame)
{
	D3D11_TEXTURE2D_DESC desc;
	frame->GetDesc(&desc);
	avcodec_register_all();
	_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (_codec == nullptr) {
		// no H264 codec found.
		cout << "No H264 codec" << endl;
		return false;
	}

	
	_context = avcodec_alloc_context3(_codec);


	_context->bit_rate = 10000000;
	_context->width = desc.Width;
	_context->height = desc.Height;
	_context->time_base = AVRational { 1, 60 };
	_context->gop_size = 100000 * 60;
	_context->max_b_frames = 0;
	_context->pix_fmt = AV_PIX_FMT_YUV420P;
	av_opt_set(_context->priv_data, "tune", "zerolatency", 0);
	av_opt_set(_context->priv_data, "preset", "ultrafast", 0);
	int err;
	if ((err = avcodec_open2(_context, _codec, nullptr)) < 0) {
		cout << "Could not open codec. " << endl;
		return false;
	}
	_picture = av_frame_alloc();
	auto ret = av_image_alloc(_picture->data, _picture->linesize, _context->width, _context->height, _context->pix_fmt, 32);
	if (ret < 0) {
		cout << "Could not alloc image" << endl;
		return false;
	}
	_picture->format = _context->pix_fmt;
	_picture->width = _context->width;
	_picture->height = _context->height;

	_swsContext = sws_getContext(_picture->width, _picture->height, AV_PIX_FMT_BGRA, _picture->width, _picture->height, AV_PIX_FMT_YUV420P, 0, 0, 0, 0);

	ID3D11Device * device; frame->GetDevice(&device);
	device->GetImmediateContext(&icontext);
	auxFrame = nullptr;
	D3D11_TEXTURE2D_DESC destDesc;
	destDesc.Format = desc.Format;
	destDesc.Height = desc.Height;
	destDesc.Width = desc.Width;
	destDesc.Usage = D3D11_USAGE_STAGING;
	destDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	destDesc.BindFlags = 0;
	destDesc.MipLevels = destDesc.ArraySize = 1;
	destDesc.SampleDesc.Quality = 0;
	destDesc.SampleDesc.Count = 1;
	destDesc.MiscFlags = 0;
	auto res = device->CreateTexture2D(&destDesc, nullptr, &auxFrame);
	
	device->Release();
	return true;
}

bool AvEncoder::EncodeFrame(ID3D11Texture2D * frame)
{
	if (_codec == nullptr) {
		if (!InitEncoder(frame)) {
			return false;
		}
	}
	::av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;

	
	D3D11_TEXTURE2D_DESC desc;
	frame->GetDesc(&desc);

	icontext->CopyResource(auxFrame, frame);

	D3D11_MAPPED_SUBRESOURCE resource;
	UINT subresource = D3D11CalcSubresource(0, 0, 0);
	icontext->Map(auxFrame, subresource, D3D11_MAP_READ_WRITE, 0, &resource);

	//std::unique_ptr<BYTE> pBuf(new BYTE[resource.RowPitch*desc.Height]);
	//UINT lBmpRowPitch = _context->width * 4;
	BYTE* sptr = reinterpret_cast<BYTE*>(resource.pData);
	
	uint8_t * inData[1] = { sptr };
	int inLineSize[1] = { 4 * _context->width };
	sws_scale(_swsContext, inData, inLineSize, 0, _context->height, _picture->data, _picture->linesize);
	{
		int got_output;
		int ret = avcodec_encode_video2(_context, &pkt, _picture, &got_output);
		if (ret < 0) {
			cout << "error encoding frame" << endl;
			return false;
		}
	}
	icontext->Unmap(auxFrame, subresource);
	return true;
}

unique_ptr<FrameBuffer> AvEncoder::PullFrame()
{
	return make_unique<AvFrameBuffer>(pkt);
}

