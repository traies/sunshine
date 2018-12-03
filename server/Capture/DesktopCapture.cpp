#include "stdafx.h"
#include "DesktopCapture.h"
#include <iostream>
#include <variant>

using namespace Capture;
using namespace std;



variant<IDXGIOutputDuplication *, sun::Error> DesktopCapture::InitIDXGIOutputDupl(ID3D11Device * device)
{
	// From DesktopDuplication sample from MSDN (DuplicationManager::InitDupl)
	// Get DXGI device
//	IDXGIDevice * device;
	return nullptr;
}

DesktopCapture::DesktopCapture(unique_ptr<Encoder> encoder,IDXGIOutputDuplication * outputDupl, SOCKET sendSock):
	_encoder(move(encoder)), _outputDupl(outputDupl), _sendSock(sendSock)
{
}

DesktopCapture::~DesktopCapture()
{
}

void DesktopCapture::GrabFrame()
{
	IDXGIResource * resource = nullptr;
	
	DXGI_OUTDUPL_FRAME_INFO info;
	int frame = 0;
	std::unique_ptr<FrameBuffer> buffer;
	while (true) {
		//auto start = std::chrono::high_resolution_clock::now();
		//while (_outputDupl->AcquireNextFrame(0, &info, &resource) != S_OK);
		auto begin = std::chrono::high_resolution_clock::now();
		
		auto res = _outputDupl->AcquireNextFrame(1, &info, &resource);
		if (res == S_OK && info.LastPresentTime.QuadPart > 0) {
			
			//std::cout << "Acumulated frames" << info.AccumulatedFrames << std::endl;
			ID3D11Texture2D * texture = nullptr;
			resource->QueryInterface<ID3D11Texture2D>(&texture);
			resource->Release();
			if (texture == nullptr) {
				//std::cout << "No texture to grab" << std::endl;
				continue;
			}
			else {
				_encoder->EncodeFrame(texture);
			}
			auto end = std::chrono::high_resolution_clock::now();
			std::cout << "Encoding latency " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;
			for (int i = 0; i < 10; i++) {
				buffer = _encoder->PullFrame();
				if (buffer == nullptr) {
					Sleep(1);
				}
				else {
					break;
				}
			}
			if (buffer == nullptr) {
				resource->Release();
				_outputDupl->ReleaseFrame();
				continue;
			}
			int start = 0, bytesLeft = buffer->size;
			while (bytesLeft > 0) {
				int sendBytes = min(WSAEMSGSIZE, bytesLeft);
				//int sent = sendto(_sendSock, reinterpret_cast<const char *>(buffer->ptr) + start, sendBytes, 0, reinterpret_cast<sockaddr *>(&_remoteAddr), sizeof(_remoteAddr));
				int sent = send(_sendSock, reinterpret_cast<const char *>(buffer->ptr) + start, sendBytes, 0);
				if (sent < 0) {
					std::cout << "Error sending bytes" << std::endl;
				}
				else {
					start += sendBytes;
					bytesLeft -= sendBytes;
				}
			}
			//}
			
			
			
		}
		if (res == S_OK) {
			resource->Release();
			_outputDupl->ReleaseFrame();
		}
		/*auto mid = std::chrono::high_resolution_clock::now();
		std::cout << "GrabFrame took " << std::chrono::duration_cast<std::chrono::microseconds>(mid - start).count() << std::endl;*/
		
		/*HRESULT hr = ;
		if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
			std::cout << "AcquireNextFrame timeouted" << std::endl;
			return;
		}*/
		
		//std::cout << "Grabbing Frame" << std::chrono::duration_cast<std::chrono::microseconds>(_current - temp).count() << std::endl;
		/*output->WaitForVBlank();*/
	}
	
}