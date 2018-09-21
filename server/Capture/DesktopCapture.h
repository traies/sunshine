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

		DesktopCapture(HDESK desktop, AmdEncoder encoder, IDXGIOutputDuplication * outputDupl);
		~DesktopCapture();
		void GrabFrame();
	private: 
		IDXGIOutputDuplication * _outputDupl;
		HDESK const _desktop;
		AmdEncoder _encoder;
		std::chrono::time_point<std::chrono::high_resolution_clock> _current; 
	};
}


