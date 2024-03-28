#include "pch.h"

#include "bot.hpp"
#include "spdlog/spdlog.h"

#include "kiero/kiero.h"
#include <d3d11.h>
#include <Windows.h>

#include "themmokhtar/d3d11/fingerprint.hpp"
#include "themmokhtar/d3d11/shaders.hpp"

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

static themmokhtar::d3d11::fingerprint::FingerprintController fingerprintController;
static SIZE_T chosenFingerprintIndex = 0;

static ID3D11PixelShader* pShaderRed = NULL;
static ID3D11Texture2D* pTextureRed = nullptr;
static ID3D11ShaderResourceView* pTextureView;
static ID3D11SamplerState* pSamplerState;

/// <summary>
/// Hook a function by its index.
/// </summary>
/// <param name="index">The index of the function to hook.</param>
/// <param name="hookFunction">The function to call instead of the original function.</param>
/// <returns>The address of the original function.</returns>
PVOID hookFunction(uint16_t index, PVOID hookFunction)
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

inline void captureFingerprint(ID3D11DeviceContext* pContext)
{
	if (fingerprintController.captureFingerprint(pContext))
		spdlog::debug("Captured a total of {} fingerprints", fingerprintController.getFingerprintCount());
}

void __stdcall hkDrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	spdlog::trace("hkDrawIndexed(): Entry");

	captureFingerprint(pContext);
	//pContext->PSSetShader(pShaderRed, NULL, NULL);
	//for (int x1 = 0; x1 <= 10; x1++)
	//	pContext->PSSetShaderResources(x1, 1, &pTextureView);
	//pContext->PSSetSamplers(0, 1, &pSamplerState);
	spdlog::trace("hkDrawIndexed(): Exit");

	oDrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
}

void __stdcall hkDraw(ID3D11DeviceContext* pContext, UINT VertexCount, UINT StartVertexLocation)
{
	spdlog::trace("hkDraw(): Entry");
	captureFingerprint(pContext);
	//pContext->PSSetShader(pShaderRed, NULL, NULL);
	spdlog::trace("hkDraw(): Exit");

	oDraw(pContext, VertexCount, StartVertexLocation);
}

void __stdcall hkDrawInstanced(ID3D11DeviceContext* pContext, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
{
	spdlog::trace("hkDrawInstanced(): Entry");
	captureFingerprint(pContext);
	spdlog::trace("hkDrawInstanced(): Exit");

	oDrawInstanced(pContext, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void __stdcall hkDrawIndexedInstanced(ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
	spdlog::trace("hkDrawIndexedInstanced(): Entry");
	captureFingerprint(pContext);
	spdlog::trace("hkDrawIndexedInstanced(): Exit");

	oDrawIndexedInstanced(pContext, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void __stdcall hkDrawAuto(ID3D11DeviceContext* pContext)
{
	spdlog::trace("hkDrawAuto(): Entry");
	captureFingerprint(pContext);
	spdlog::trace("hkDrawAuto(): Exit");

	oDrawAuto(pContext);
}

void __stdcall hkDrawIndexedInstancedIndirect(ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
	spdlog::trace("hkDrawIndexedInstancedIndirect(): Entry");
	captureFingerprint(pContext);
	spdlog::trace("hkDrawIndexedInstancedIndirect(): Exit");

	oDrawIndexedInstancedIndirect(pContext, pBufferForArgs, AlignedByteOffsetForArgs);
}

void __stdcall hkDrawInstancedIndirect(ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
	spdlog::trace("hkDrawInstancedIndirect(): Entry");
	captureFingerprint(pContext);
	spdlog::trace("hkDrawInstancedIndirect(): Exit");

	oDrawInstancedIndirect(pContext, pBufferForArgs, AlignedByteOffsetForArgs);
}

long __stdcall hkPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	//spdlog::trace("hkPresent11(): Entry"); // This is commented out because it's too verbose
	static bool drawFunctionsHooked = false;
	
	ID3D11Device* pDevice;
	pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice);

	ID3D11DeviceContext* pContext;
	pDevice->GetImmediateContext(&pContext);

	if (pShaderRed == NULL)
	{
		themmokhtar::d3d11::shaders::GenerateShaderRgb(pDevice, &pShaderRed, 1.0f, 0.0f, 0.0f);

		// Generate a red texture
		static const uint32_t color = 0xff0000ff;
		D3D11_SUBRESOURCE_DATA initData = { &color, sizeof(uint32_t), 0 };
		D3D11_TEXTURE2D_DESC desc;
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		memset(&desc, 0, sizeof(desc));
		desc.Width = 1;
		desc.Height = 1;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		HRESULT hr;
		hr = pDevice->CreateTexture2D(&desc, &initData, &pTextureRed);
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateTexture2D failed");
			//return hr;
		}

		// Generate the resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		memset(&SRVDesc, 0, sizeof(SRVDesc));
		SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = pDevice->CreateShaderResourceView(pTextureRed, &SRVDesc, &pTextureView);
		if (FAILED(hr))
		{
			pTextureRed->Release();
			throw std::runtime_error("CreateShaderResourceView failed");
			//return hr;
		}
	}

	if (!drawFunctionsHooked)
	{
		kiero::updateMethodsTable::d3d11(pSwapChain, pDevice, pContext);
		drawFunctionsHooked = true;

		oDrawIndexed = (DrawIndexed)hookFunction(D3D11_DRAWINDEXED_INDEX, (PVOID)hkDrawIndexed);
		oDraw = (Draw)hookFunction(D3D11_DRAW_INDEX, (PVOID)hkDraw);
		oDrawIndexedInstanced = (DrawIndexedInstanced)hookFunction(D3D11_DRAWINDEXEDINSTANCED_INDEX, (PVOID)hkDrawIndexedInstanced);
		oDrawInstanced = (DrawInstanced)hookFunction(D3D11_DRAWINSTANCED_INDEX, (PVOID)hkDrawInstanced);
		oDrawAuto = (DrawAuto)hookFunction(D3D11_DRAWAUTO_INDEX, (PVOID)hkDrawAuto);
		oDrawIndexedInstancedIndirect = (DrawIndexedInstancedIndirect)hookFunction(D3D11_DRAWINDEXEDINSTANCEDINDIRECT_INDEX, (PVOID)hkDrawIndexedInstancedIndirect);
		oDrawInstancedIndirect = (DrawInstancedIndirect)hookFunction(D3D11_DRAWINSTANCEDINDIRECT_INDEX, (PVOID)hkDrawInstancedIndirect);
	}

	bool fingerprintUpdated = false;
	if (GetAsyncKeyState(VK_PRIOR) & 1)
	{
		spdlog::trace("Previous fingerprint");
		if (chosenFingerprintIndex == 0)
			chosenFingerprintIndex = fingerprintController.getFingerprintCount() - 1;
		else
			chosenFingerprintIndex--;

		fingerprintUpdated = true;
	}
	if (GetAsyncKeyState(VK_NEXT) & 1)
	{
		spdlog::trace("Next fingerprint");
		if (chosenFingerprintIndex == fingerprintController.getFingerprintCount() - 1)
			chosenFingerprintIndex = 0;
		else
			chosenFingerprintIndex++;

		fingerprintUpdated = true;
	}

	if (fingerprintUpdated)
	{
		spdlog::info("Chosen fingerprint ({}/{}) information:", chosenFingerprintIndex+1, fingerprintController.getFingerprintCount());
		themmokhtar::d3d11::fingerprint::ModelFingerprint fingerprint = fingerprintController.getFingerprintAt(chosenFingerprintIndex);

		spdlog::info("\t Vertex stride: {}", fingerprint.vertexStride);
		spdlog::info("\t Vertex byte width: {}", fingerprint.vertexByteWidth);
		spdlog::info("\t Index byte width: {}", fingerprint.indexByteWidth);
		spdlog::info("\t Constant byte width: {}", fingerprint.constantByteWidth);
	}

	//spdlog::trace("hkPresent11(): Exit"); // This is commented out because it's too verbose

	return oPresent(pSwapChain, SyncInterval, Flags);
}

void botMain()
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
	oPresent = (Present)hookFunction(D3D11_PRESENT_INDEX, (PVOID)hkPresent11);

	// Wait for the message box to be closed
	MessageBoxA(nullptr, "botMain: Press OK to close bot", "Redmatch2Bot", MB_OK);

	spdlog::trace("{}: Exit", __func__);
}

void botCleanup()
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
