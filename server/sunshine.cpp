// sunshine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include "Capture/DesktopCapture.h"
#include <iostream>
#include <functional>
#include <thread>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

using namespace std::chrono;

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

int main()
{
	auto const desktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
	if (desktop == nullptr) {
		// No desktop access.
		std::cout << "Error: no desktop access." << std::endl;
		return 1;
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
		std::cout << "Device creation failed.";
		return 1;
	}

	Capture::DesktopCapture capture(desktop, device);
	auto temp = std::bind(&Capture::DesktopCapture::GrabFrame, &capture);
	Repeat(temp, microseconds(16700), microseconds(1200));

    return 0;
}

