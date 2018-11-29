#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <chrono>
#include "Encoder/AmdEncoder.h"
#include <dxgi1_6.h>

namespace Capture {
	class DesktopCapture
	{
	public:
		static std::variant<IDXGIOutputDuplication *, sun::Error> InitIDXGIOutputDupl(ID3D11Device * device);

		DesktopCapture(std::unique_ptr<Encoder> encoder, IDXGIOutputDuplication * outputDupl, SOCKET sendSock, sockaddr_in remoteAddr);
		~DesktopCapture();
		void GrabFrame();
	private: 
		IDXGIOutputDuplication * _outputDupl;
		std::unique_ptr<Encoder> _encoder;
		SOCKET _sendSock;
		sockaddr_in _remoteAddr;
		std::chrono::time_point<std::chrono::high_resolution_clock> _current; 
	};
}


