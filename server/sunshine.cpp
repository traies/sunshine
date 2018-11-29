// sunshine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include "Capture/DesktopCapture.h"
#include <iostream>
#include <functional>
#include <thread>
#include <chrono>
#include <d3d11.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "common")

using namespace std::chrono;
using namespace std;

void Repeat(const std::function<void()> task, const high_resolution_clock::duration sleepy_time, const high_resolution_clock::duration spin_lock_time)
{
	auto last = high_resolution_clock::now();
	do
	{
		const auto current = high_resolution_clock::now();
		const auto difference = current - last;
		if ( difference >= sleepy_time)
		{
			task();
			last = current;
		} else if (sleepy_time - difference > spin_lock_time)
		{
			std::this_thread::sleep_for(sleepy_time - difference - spin_lock_time);
		}
	} while (true);
}

 variant<tuple<HDESK, ID3D11Device *>, sun::Error> InitDX11Device()
{
	auto const desktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
	if (desktop == nullptr) {
		return sun::Error("No desktop access.");
	}

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_9_1
	};
	ID3D11Device * device;
	ID3D11DeviceContext * context;
	D3D_FEATURE_LEVEL featureLevel;
	auto hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&device,
		&featureLevel,
		&context
	);

	if (FAILED(hr)) {
		return sun::Error("Device creation failed.");
	}
	return tuple(desktop, device);
}

 int main()
{
	auto desktopInit = InitDX11Device();
	if (holds_alternative<sun::Error>(desktopInit)) {
		auto ex = get<sun::Error>(desktopInit);
		std::cerr << ex.Message() << std::endl;
		return 1;
	}
	
	auto[desktop, device] = get<tuple<HDESK, ID3D11Device *>>(desktopInit);
	auto variant = AmdEncoder::NewEncoder(device);
	if (holds_alternative<sun::Error>(variant)) {
		auto ex = get<sun::Error>(variant);
		std::cerr << ex.Message() << std::endl;
		return 1;
	}
	// Duplicate output. TODO proper error handling.
	IDXGIDevice * dxgiDevice; device->QueryInterface<IDXGIDevice>(&dxgiDevice);
	IDXGIAdapter * adapter; dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void **> (&adapter)); 
	IDXGIOutput * output; adapter->EnumOutputs(0, &output); adapter->Release();
	IDXGIOutput1 * output1; output->QueryInterface<IDXGIOutput1>(&output1);
	IDXGIOutputDuplication * outputDuplication;  output1->DuplicateOutput(device, &outputDuplication); output1->Release();


	
	auto encoder = get<AmdEncoder>(variant);

	WSADATA wsaData;
	int ires = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ires != 0) {
		std::cout << "Winsock2 startup failed" << std::endl;
		return 1;
	}
	sockaddr_in remoteAddrinfo;
	remoteAddrinfo.sin_family = AF_INET;
	remoteAddrinfo.sin_addr.s_addr = inet_addr("127.0.0.1");
	remoteAddrinfo.sin_port = htons(1234);
	SOCKET sendSock = socket(AF_UNSPEC, SOCK_DGRAM, IPPROTO_UDP);


	Capture::DesktopCapture capture(move(encoder), outputDuplication, sendSock, remoteAddrinfo);
	
	capture.GrabFrame();
    return 0;
}

