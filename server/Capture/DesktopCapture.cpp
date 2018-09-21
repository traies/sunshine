#include "stdafx.h"
#include "DesktopCapture.h"
#include <iostream>
#include <variant>

using namespace Capture;
using namespace std;



//variant<IDXGIOutputDuplication *, sun::Error> DesktopCapture::InitIDXGIOutputDupl(ID3D11Device * device)
//{
//	// From DesktopDuplication sample from MSDN (DuplicationManager::InitDupl)
//	// Get DXGI device
//	IDXGIDevice * device;
//
//}

DesktopCapture::DesktopCapture(HDESK desktop, AmdEncoder encoder, IDXGIOutputDuplication * outputDupl):
	_desktop(desktop), _encoder(encoder), _outputDupl(outputDupl)
{

}

DesktopCapture::~DesktopCapture()
{
}

void DesktopCapture::GrabFrame()
{
	auto temp = _current;
	_current = std::chrono::high_resolution_clock::now();
	
	std::cout << "Grabbing Frame" << std::chrono::duration_cast<std::chrono::microseconds>(_current - temp).count() << std::endl;
}