#include "pch.h"

#include "fingerprint.hpp"
#include <stdexcept>

bool themmokhtar::d3d11::fingerprint::operator==(const ModelFingerprint& lhs, const ModelFingerprint& rhs)
{
	bool result = true;

	result &= lhs.vertexStride == rhs.vertexStride;
	result &= lhs.vertexByteWidth == rhs.vertexByteWidth;
	result &= lhs.indexByteWidth == rhs.indexByteWidth;
	result &= lhs.constantByteWidth == rhs.constantByteWidth;

	return result;
}

namespace std {
	template<> struct hash<themmokhtar::d3d11::fingerprint::ModelFingerprint>
	{
		std::size_t operator()(const themmokhtar::d3d11::fingerprint::ModelFingerprint& obj) const noexcept
		{
			std::size_t h1 = std::hash<int>{}(obj.vertexStride);
			std::size_t h2 = std::hash<int>{}(obj.vertexByteWidth);
			std::size_t h3 = std::hash<int>{}(obj.indexByteWidth);
			std::size_t h4 = std::hash<int>{}(obj.constantByteWidth);
			return (h1 ^ h3 + h4) ^ (h2 << 1);
		}
	};
}

themmokhtar::d3d11::fingerprint::ModelFingerprint themmokhtar::d3d11::fingerprint::FingerprintController::getModelFingerprint(ID3D11DeviceContext* pContext)
{
	struct ModelFingerprint modelFingerprint = { 0 };

	// Get the vertex buffer stride and byte width
	ID3D11Buffer* pVertexBuffer;
	D3D11_BUFFER_DESC vertexBufferDesc;
	UINT veBufferOffset = 0;

	pContext->IAGetVertexBuffers(0, 1, &pVertexBuffer, &modelFingerprint.vertexStride, &veBufferOffset);
	if (pVertexBuffer)
	{
		pVertexBuffer->GetDesc(&vertexBufferDesc);
		modelFingerprint.vertexByteWidth = vertexBufferDesc.ByteWidth;
	}
	if (pVertexBuffer != NULL)
	{
		pVertexBuffer->Release();
		pVertexBuffer = NULL;
	}

	// Get the index buffer byte width
	ID3D11Buffer* pIndexBuffer;
	D3D11_BUFFER_DESC indexBufferDesc;
	DXGI_FORMAT indexFormat;
	UINT indexBufferOffset = 0;

	pContext->IAGetIndexBuffer(&pIndexBuffer, &indexFormat, &indexBufferOffset);
	if (pIndexBuffer)
	{
		pIndexBuffer->GetDesc(&indexBufferDesc);
		modelFingerprint.indexByteWidth = indexBufferDesc.ByteWidth;
	}
	if (pIndexBuffer != NULL)
	{
		pIndexBuffer->Release();
		pIndexBuffer = NULL;
	}

	// Get the constant buffer byte width
	ID3D11Buffer* pConstantBuffer;
	D3D11_BUFFER_DESC constantBufferDesc;

	pContext->VSGetConstantBuffers(0, 1, &pConstantBuffer);
	if (pConstantBuffer)
	{
		pConstantBuffer->GetDesc(&constantBufferDesc);
		modelFingerprint.constantByteWidth = constantBufferDesc.ByteWidth;
	}
	if (pConstantBuffer != NULL)
	{
		pConstantBuffer->Release();
		pConstantBuffer = NULL;
	}

	return modelFingerprint;
}

themmokhtar::d3d11::fingerprint::FingerprintController::FingerprintController()
{
	fingerprints = new std::unordered_set<ModelFingerprint>();
}

themmokhtar::d3d11::fingerprint::FingerprintController::~FingerprintController()
{
	delete fingerprints;
}

bool themmokhtar::d3d11::fingerprint::FingerprintController::captureFingerprint(ID3D11DeviceContext* context)
{
	ModelFingerprint modelFingerprint = getModelFingerprint(context);
	auto result = fingerprints->insert(modelFingerprint);

	return result.second;
}

void themmokhtar::d3d11::fingerprint::FingerprintController::clearFingerprints()
{
	fingerprints->clear();
}

bool themmokhtar::d3d11::fingerprint::FingerprintController::hasFingerprint(ID3D11DeviceContext* context) const
{
	ModelFingerprint modelFingerprint = getModelFingerprint(context);
	return fingerprints->find(modelFingerprint) != fingerprints->end();
}

themmokhtar::d3d11::fingerprint::ModelFingerprint themmokhtar::d3d11::fingerprint::FingerprintController::getFingerprintAt(SIZE_T index) const
{
	if (index >= fingerprints->size())
		throw std::out_of_range("Index out of range");

	auto it = fingerprints->begin();
	std::advance(it, index);
	return *it;
}

SIZE_T themmokhtar::d3d11::fingerprint::FingerprintController::getFingerprintCount() const
{
	return fingerprints->size();
}
