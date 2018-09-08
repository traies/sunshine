#include "stdafx.h"
#include "DesktopCapture.h"
#include <iostream>

using namespace Capture;

DesktopCapture::DesktopCapture(HDESK desktop, ID3D11Device * device): 
	_desktop(desktop), _device(device)
{
	_device->AddRef();
}


DesktopCapture::~DesktopCapture()
{
	_device->Release();
}

void DesktopCapture::GrabFrame()
{
	auto temp = _current;
	_current = std::chrono::high_resolution_clock::now();
	std::cout << "Grabbing Frame" << std::chrono::duration_cast<std::chrono::microseconds>(_current - temp).count() << std::endl;
}