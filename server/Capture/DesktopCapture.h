#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <chrono>

namespace Capture {
	class DesktopCapture
	{
	public:
		DesktopCapture(HDESK desktop, ID3D11Device * device);
		~DesktopCapture();
		void GrabFrame();
	private: 
		HDESK const _desktop;
		ID3D11Device * const _device;
		std::chrono::time_point<std::chrono::high_resolution_clock> _current; 
	};
}


