#include "pch.h"

#include "shaders.hpp"
#include <d3dcompiler.h>

// Source: https://github.com/niemand-sec/DirectX11Hook/blob/0dc86cf261ed09087f5b7ee53525f9e2b892e7a7/DirectX11Hook/DirectX11Hook.cpp#L196
// raiders posted this here - http://www.unknowncheats.me/forum/direct3d/65135-directx-10-generateshader.html
HRESULT themmokhtar::d3d11::shaders::GenerateShaderRgb(ID3D11Device* pD3DDevice, ID3D11PixelShader** pShader, float r, float g, float b)
{
	char szCast[] = "struct VS_OUT"
		"{"
		"    float4 Position   : SV_Position;"
		"    float4 Color    : COLOR0;"
		"};"

		"float4 main( VS_OUT input ) : SV_Target"
		"{"
		"    float4 fake;"
		"    fake.a = 1.0;"
		"    fake.r = %f;"
		"    fake.g = %f;"
		"    fake.b = %f;"
		"    return fake;"
		"}";
	ID3D10Blob* pBlob;
	char szPixelShader[1000];
	//sprintf(szPixelShader, szCast, r, g, b);

	HRESULT hr = D3DCompile(szPixelShader, sizeof(szPixelShader), "shader", NULL, NULL, "main", "ps_4_0", NULL, NULL, &pBlob, NULL);

	if (FAILED(hr))
		return hr;

	hr = pD3DDevice->CreatePixelShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, pShader);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

