#include "pch.h"

#include "bot.hpp"
#include "spdlog/spdlog.h"

#include "kiero.h"
#include <d3d11.h>
#include <Windows.h>

typedef long(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
static Present oPresent = NULL;

static bool kieroInitialized = false; 
static bool functionHooked = false;
static bool cleanupCalled = false;

long __stdcall hkPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	spdlog::trace("hkPresent11(): Entry");
	spdlog::trace("hkPresent11(): Exit");

	return oPresent(pSwapChain, SyncInterval, Flags);
}

void BotMain()
{
	kiero::Status::Enum status;
	
	spdlog::trace("{}: Entry", __func__);

	// Initialize kiero
	if ((status = kiero::init(kiero::RenderType::D3D11)) != kiero::Status::Success)
	{
		spdlog::error("{}: kiero::init failed: {}", __func__, (ULONG)-status);
		throw std::runtime_error("kiero::init failed");
	}
	kieroInitialized = true;

	// Hook the Present function
	if ((status = kiero::bind(8, (void**)&oPresent, hkPresent11)) != kiero::Status::Success)
	{
		spdlog::error("{}: kiero::bind failed: {}", __func__, (ULONG)-status);
		throw std::runtime_error("kiero::bind failed");
	}
	functionHooked = true;

	spdlog::info("{}: Hooked Present function", __func__);
	spdlog::debug("{}: Original Present function address: {:#X}", __func__, (PVOID)oPresent);


	// Wait for the message box to be closed
	MessageBoxA(nullptr, "BotMain: Press OK to close bot", "Redmatch2Bot", MB_OK);

	spdlog::trace("{}: Exit", __func__);
}

void BotCleanup()
{
	if (cleanupCalled)
		return;
	
	cleanupCalled = true;

	spdlog::trace("{}: Entry", __func__);

	if (functionHooked)
	{
		functionHooked = false;
		kiero::unbind(kiero::RenderType::D3D11);

		spdlog::info("{}: Unhooked Present function", __func__);
	}

	if (kieroInitialized)
	{
		kieroInitialized = false;
		kiero::shutdown();

		spdlog::info("{}: kiero::shutdown", __func__);
	}

	spdlog::trace("{}: Exit", __func__);
}


//#include "pch.h"
//
//#include "bot.hpp"
//#include "spdlog/spdlog.h"
//#include "spdlog/sinks/basic_file_sink.h"
//
//#include <Windows.h>
//#include <d3d11.h>
//
//#include "D3D11Hooking.hpp"
//
////#include <wrl/client.h>
////
////using namespace Microsoft::WRL;
//
//// https://github.com/ExtraConcentratedJuice/ChristWareAmongUs/blob/3a21a3a99298819629314e835a82fdb15517bf4d/user/D3D11Hooking.cpp
//PVOID FindSwapChainPresent()
//{
//	HWND gameWindow = FindWindow(NULL, L"Redmatch 2");
//	if (gameWindow == NULL)
//		throw std::runtime_error("FindWindow failed: " + std::to_string(GetLastError()));
//
//	//// Create a device (https://walbourn.github.io/anatomy-of-direct3d-11-create-device/)
//	//ComPtr<ID3D11Device> device;
//	//ComPtr<ID3D11DeviceContext> context;
//	//D3D_FEATURE_LEVEL fl;
//	//HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
//	//	nullptr, D3D11_CREATE_DEVICE_DEBUG, nullptr,
//	//	0, D3D11_SDK_VERSION, &device, &fl, &context);
//	
//
//	//D3DPRESENT_PARAMETERS d3dpp;
//	//ZeroMemory(&d3dpp, sizeof(d3dpp));
//	//d3dpp.Windowed = TRUE;
//	//d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
//	//d3dpp.hDeviceWindow = gameWindow;
//	//LPDIRECT3DDEVICE9 device;
//	//HRESULT res = pD3D->CreateDevice(
//	//	D3DADAPTER_DEFAULT,
//	//	D3DDEVTYPE_HAL,
//	//	gameWindow,
//	//	D3DCREATE_SOFTWARE_VERTEXPROCESSING,
//	//	&d3dpp, &device);
//	//if (FAILED(res)) return 0
//	return 0;
//}
//
//void BotMain()
//{
//	spdlog::info("BotMain(): Entry");
//
//	D3D_PRESENT_FUNCTION presentFunction = GetD3D11PresentFunction();
//
//	spdlog::info("BotMain(): Exit");
//}
//
