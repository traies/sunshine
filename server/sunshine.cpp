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
#include "Encoder\NvidiaEncoder.h"
#include "Encoder\AvEncoder.h"
//#define inline __inline

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "common")

#define SOFTWARE_ENC
//#define NVIDIA

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

int main(int argc, const char* argv[])
{
	if (argc < 2) {
		std::cout << "Missing parameters. Format: <port>";
		return 1;
	};
	auto desktopInit = InitDX11Device();
	if (holds_alternative<sun::Error>(desktopInit)) {
		auto ex = get<sun::Error>(desktopInit);
		std::cerr << ex.Message() << std::endl;
		return 1;
	}
	
	auto[desktop, device] = get<tuple<HDESK, ID3D11Device *>>(desktopInit);
	
	// Duplicate output. TODO proper error handling.
	IDXGIDevice * dxgiDevice; device->QueryInterface<IDXGIDevice>(&dxgiDevice);
	IDXGIAdapter * adapter; dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void **> (&adapter)); 
	IDXGIOutput * output; adapter->EnumOutputs(0, &output); adapter->Release();
	IDXGIOutput1 * output1; output->QueryInterface<IDXGIOutput1>(&output1);
	IDXGIOutputDuplication * outputDuplication; 
	{
		HRESULT res = output1->DuplicateOutput(device, &outputDuplication);
		if (res != S_OK) {
			cout << "Error duplicating output" << endl;
			return 1;
		}
	}
	output1->Release();


#ifdef SOFTWARE_ENC
	auto encoder = make_unique<AvEncoder>();
#elif NVIDIA
	auto encoder = make_unique<NvidiaEncoder>();
#else

	auto variant = AmdEncoder::NewEncoder(device);
	if (holds_alternative<sun::Error>(variant)) {
		auto ex = get<sun::Error>(variant);
		std::cerr << ex.Message() << std::endl;
		return 1;
	}
	auto encoder = move(get<unique_ptr<Encoder>>(variant));

#endif

	WSADATA wsaData;
	int ires = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ires != 0) {
		std::cout << "Winsock2 startup failed" << std::endl;
		return 1;
	}

	
	addrinfo * addrInfo, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	{
		int res = getaddrinfo(NULL, argv[1], &hints, &addrInfo);
		if (res != 0) {
			cout << "Could not get addr" << endl;
			return 1;
		}
	}
	
	SOCKET sendSock = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
	{
		int res = ::bind(sendSock, addrInfo->ai_addr, addrInfo->ai_addrlen);
		if (res != 0) {
			std::cout << "Could not connect. " << WSAGetLastError() << std::endl;
			return 1;
		}
	}
	freeaddrinfo(addrInfo);
	std::cout << "Binded..." << std::endl;
	{
		int res = ::listen(sendSock, SOMAXCONN);
		if (res == SOCKET_ERROR) {
			std::cout << "Could not listen. " << WSAGetLastError() << std::endl;
			return 1;
		}
	}

	std::cout << "Accepting..." << std::endl;
	{
		sendSock = ::accept(sendSock, nullptr, 0);
		if (sendSock == 0) {
			std::cout << "Could not accept. " << WSAGetLastError() << std::endl;
			return 1;
		}
	}
	std::cout << "Accepted..." << std::endl;


	Capture::DesktopCapture capture(move(encoder), outputDuplication, sendSock);
	
	capture.GrabFrame();
	
	closesocket(sendSock);
    return 0;
}

