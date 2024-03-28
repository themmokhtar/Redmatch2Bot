#pragma once

#include <d3d11.h>

namespace themmokhtar
{
	namespace d3d11
	{
		namespace shaders
		{
			/// <summary>
			/// Generates a pixel shader that outputs a solid color.
			/// </summary>
			/// <param name="pD3DDevice">The Direct3D device.</param>
			/// <param name="pShader">The pixel shader to generate.</param>
			/// <param name="r">The red component of the color.</param>
			/// <param name="g">The green component of the color.</param>
			/// <param name="b">The blue component of the color.</param>
			/// <returns>The result of the operation.</returns>
			HRESULT GenerateShaderRgb(ID3D11Device* pD3DDevice, ID3D11PixelShader** pShader, float r, float g, float b);
		}
	}
}
