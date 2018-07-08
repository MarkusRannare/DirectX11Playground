#pragma once

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

/**
 * Mirror of HLSL DirectionalLight
 * Added padding to match hlsl vector packing
 */
struct DirectionalLight
{
	DirectX::XMFLOAT3	DiffuseColor;
	float				_pad0;
	DirectX::XMFLOAT3	SpecularColor;
	float				_pad1;
	DirectX::XMFLOAT3	Direction;
	float				_pad2;
};