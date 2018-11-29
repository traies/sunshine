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

DesktopCapture::DesktopCapture(unique_ptr<Encoder> encoder,IDXGIOutputDuplication * outputDupl, SOCKET sendSock, sockaddr_in remoteAddr):
	_encoder(move(encoder)), _outputDupl(outputDupl), _sendSock(sendSock), _remoteAddr(remoteAddr)
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
		_outputDupl->ReleaseFrame();
		auto res = _outputDupl->AcquireNextFrame(1, &info, &resource);
		if (res == S_OK) {
			
			//std::cout << "Acumulated frames" << info.AccumulatedFrames << std::endl;
			ID3D11Texture2D * texture = nullptr;
			resource->QueryInterface<ID3D11Texture2D>(&texture);
			resource->Release();
			if (texture == nullptr || info.AccumulatedFrames == 0) {
				//std::cout << "No texture to grab" << std::endl;
				continue;
			}
			else {
				_encoder->EncodeFrame(texture);
			}
			
			while ((buffer = _encoder->PullFrame()) == nullptr) { Sleep(1); }
			int start = 0, bytesLeft = buffer->size;
			while (bytesLeft > 0) {
				int sendBytes = min(WSAEMSGSIZE, bytesLeft);
				int sent = sendto(_sendSock, reinterpret_cast<const char *>(buffer->ptr) + start, sendBytes, 0, reinterpret_cast<sockaddr *>(&_remoteAddr), sizeof(_remoteAddr));
				if (sent < 0) {
					std::cout << "Error sending bytes" << std::endl;
				}
				else {
					start += sendBytes;
					bytesLeft -= sendBytes;
				}
			}
			//}
			resource->Release();
			auto end = std::chrono::high_resolution_clock::now();
			std::cout << "Encoding latency " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl; 
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