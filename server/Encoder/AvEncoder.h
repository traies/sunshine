#pragma once
#include "Encoder\Encoder.h"
extern "C"
{
	#include <libavcodec\avcodec.h>
	#include <libavutil\imgutils.h>
	#include <libavutil\opt.h>
	#include <libswscale\swscale.h>
};


class AvFrameBuffer : public FrameBuffer
{
public:
	AvFrameBuffer(AVPacket packet): pkt(packet)
	{
		ptr = packet.data;
		size = packet.size;
	}
	~AvFrameBuffer()
	{
		av_free_packet(&pkt);
	}
private:
	AVPacket pkt;
};

class AvEncoder :
	public Encoder
{
public:
	AvEncoder();
	~AvEncoder();

	bool EncodeFrame(ID3D11Texture2D * frame);
	std::unique_ptr<FrameBuffer> PullFrame();

private:
	bool InitEncoder(ID3D11Texture2D * frame);
	AVCodec * _codec;
	AVFrame * _picture;
	AVCodecContext * _context;
	AVPacket pkt;
	SwsContext * _swsContext;
	ID3D11DeviceContext* icontext;
	ID3D11Texture2D* auxFrame = nullptr;
};

