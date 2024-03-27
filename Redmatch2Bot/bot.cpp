#include "pch.h"

#include "bot.hpp"
#include "spdlog/spdlog.h"

#include "kiero.h"
#include <d3d11.h>
#include <Windows.h>

const uint16_t D3D11_PRESENT_INDEX = 8;
const uint16_t D3D11_DRAWINDEXED_INDEX = 73;
const uint16_t D3D11_DRAW_INDEX = 74;
const uint16_t D3D11_DRAWINDEXEDINSTANCED_INDEX = 81;
const uint16_t D3D11_DRAWINSTANCED_INDEX = 82;
const uint16_t D3D11_DRAWAUTO_INDEX = 99;
const uint16_t D3D11_DRAWINDEXEDINSTANCEDINDIRECT_INDEX = 100;
const uint16_t D3D11_DRAWINSTANCEDINDIRECT_INDEX = 101;

typedef long(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
static Present oPresent = NULL;

typedef void(__stdcall* DrawIndexed)(ID3D11DeviceContext*, UINT, UINT, INT);
static DrawIndexed oDrawIndexed = NULL;

typedef void(__stdcall* Draw)(ID3D11DeviceContext*, UINT, UINT);
static Draw oDraw = NULL;

typedef void(__stdcall* DrawIndexedInstanced)(ID3D11DeviceContext*, UINT, UINT, UINT, INT, UINT);
static DrawIndexedInstanced oDrawIndexedInstanced = NULL;

typedef void(__stdcall* DrawInstanced)(ID3D11DeviceContext*, UINT, UINT, UINT, UINT);
static DrawInstanced oDrawInstanced = NULL;

typedef void(__stdcall* DrawAuto)(ID3D11DeviceContext*);
static DrawAuto oDrawAuto = NULL;

typedef void(__stdcall* DrawIndexedInstancedIndirect)(ID3D11DeviceContext*, ID3D11Buffer*, UINT);
static DrawIndexedInstancedIndirect oDrawIndexedInstancedIndirect = NULL;

typedef void(__stdcall* DrawInstancedIndirect)(ID3D11DeviceContext*, ID3D11Buffer*, UINT);
static DrawInstancedIndirect oDrawInstancedIndirect = NULL;

static std::vector<uint16_t> hookedIndices;

static bool kieroInitialized = false;
static bool cleanupCalled = false;

long __stdcall hkPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	spdlog::trace("hkPresent11(): Entry");

	ID3D11Device* device;
	pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&device);

	spdlog::trace("hkPresent11(): Exit");

	return oPresent(pSwapChain, SyncInterval, Flags);
}

void __stdcall hkDrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	spdlog::trace("hkDrawIndexed(): Entry");

	spdlog::debug("hkDrawIndexed(): IndexCount: {}", IndexCount);

	spdlog::trace("hkDrawIndexed(): Exit");

	oDrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
}

void __stdcall hkDraw(ID3D11DeviceContext* pContext, UINT VertexCount, UINT StartVertexLocation)
{
	spdlog::trace("hkDraw(): Entry");

	spdlog::debug("hkDraw(): VertexCount: {}", VertexCount);

	spdlog::trace("hkDraw(): Exit");

	oDraw(pContext, VertexCount, StartVertexLocation);
}

void __stdcall hkDrawInstanced(ID3D11DeviceContext* pContext, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
{
	spdlog::trace("hkDrawInstanced(): Entry");

	spdlog::debug("hkDrawInstanced(): VertexCountPerInstance: {}", VertexCountPerInstance);
	spdlog::debug("hkDrawInstanced(): InstanceCount: {}", InstanceCount);
	spdlog::debug("hkDrawInstanced(): StartVertexLocation: {}", StartVertexLocation);
	spdlog::debug("hkDrawInstanced(): StartInstanceLocation: {}", StartInstanceLocation);

	spdlog::trace("hkDrawInstanced(): Exit");

	oDrawInstanced(pContext, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void __stdcall hkDrawIndexedInstanced(ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
	spdlog::trace("hkDrawIndexedInstanced(): Entry");

	spdlog::debug("hkDrawIndexedInstanced(): IndexCountPerInstance: {}", IndexCountPerInstance);
	spdlog::debug("hkDrawIndexedInstanced(): InstanceCount: {}", InstanceCount);
	spdlog::debug("hkDrawIndexedInstanced(): StartIndexLocation: {}", StartIndexLocation);
	spdlog::debug("hkDrawIndexedInstanced(): BaseVertexLocation: {}", BaseVertexLocation);
	spdlog::debug("hkDrawIndexedInstanced(): StartInstanceLocation: {}", StartInstanceLocation);

	spdlog::trace("hkDrawIndexedInstanced(): Exit");

	oDrawIndexedInstanced(pContext, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void __stdcall hkDrawAuto(ID3D11DeviceContext* pContext)
{
	spdlog::trace("hkDrawAuto(): Entry");

	spdlog::debug("hkDrawAuto(): DrawAuto");

	spdlog::trace("hkDrawAuto(): Exit");

	oDrawAuto(pContext);
}

void __stdcall hkDrawIndexedInstancedIndirect(ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
	spdlog::trace("hkDrawIndexedInstancedIndirect(): Entry");

	spdlog::debug("hkDrawIndexedInstancedIndirect(): AlignedByteOffsetForArgs: {}", AlignedByteOffsetForArgs);

	spdlog::trace("hkDrawIndexedInstancedIndirect(): Exit");

	oDrawIndexedInstancedIndirect(pContext, pBufferForArgs, AlignedByteOffsetForArgs);
}

void __stdcall hkDrawInstancedIndirect(ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
	spdlog::trace("hkDrawInstancedIndirect(): Entry");

	spdlog::debug("hkDrawInstancedIndirect(): AlignedByteOffsetForArgs: {}", AlignedByteOffsetForArgs);

	spdlog::trace("hkDrawInstancedIndirect(): Exit");

	oDrawInstancedIndirect(pContext, pBufferForArgs, AlignedByteOffsetForArgs);
}

/// <summary>
/// Hook a function by its index.
/// </summary>
/// <param name="index">The index of the function to hook.</param>
/// <param name="hookFunction">The function to call instead of the original function.</param>
/// <returns>The address of the original function.</returns>
PVOID HookFunction(uint16_t index, PVOID hookFunction)
{
	kiero::Status::Enum status;
	PVOID originalFunction;

	if ((status = kiero::bind(index, &originalFunction, hookFunction)) != kiero::Status::Success)
	{
		spdlog::error("{}: kiero::bind failed: {}", __func__, (ULONG)-status);
		throw std::runtime_error("kiero::bind failed");
	}
	hookedIndices.push_back(index);
	spdlog::info("{}: Hooked index: {}", __func__, index);
	spdlog::debug("{}: Original function address: {}", __func__, (PVOID)originalFunction);

	return originalFunction;
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

	// Hook the functions
	oPresent = (Present)HookFunction(D3D11_PRESENT_INDEX, (PVOID)hkPresent11);
	oDrawIndexed = (DrawIndexed)HookFunction(D3D11_DRAWINDEXED_INDEX, (PVOID)hkDrawIndexed);
	oDraw = (Draw)HookFunction(D3D11_DRAW_INDEX, (PVOID)hkDraw);
	oDrawIndexedInstanced = (DrawIndexedInstanced)HookFunction(D3D11_DRAWINDEXEDINSTANCED_INDEX, (PVOID)hkDrawIndexedInstanced);
	oDrawInstanced = (DrawInstanced)HookFunction(D3D11_DRAWINSTANCED_INDEX, (PVOID)hkDrawInstanced);
	oDrawAuto = (DrawAuto)HookFunction(D3D11_DRAWAUTO_INDEX, (PVOID)hkDrawAuto);
	oDrawIndexedInstancedIndirect = (DrawIndexedInstancedIndirect)HookFunction(D3D11_DRAWINDEXEDINSTANCEDINDIRECT_INDEX, (PVOID)hkDrawIndexedInstancedIndirect);
	oDrawInstancedIndirect = (DrawInstancedIndirect)HookFunction(D3D11_DRAWINSTANCEDINDIRECT_INDEX, (PVOID)hkDrawInstancedIndirect);

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

	for (uint16_t index : hookedIndices)
	{
		kiero::unbind(index);
		spdlog::trace("{}: Unhooked index: {}", __func__, index);
	}
	hookedIndices.clear();

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
