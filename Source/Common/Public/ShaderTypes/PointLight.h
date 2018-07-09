#pragma once

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

/**
* Mirror of HLSL PointLight in Lighting.hlsl
* Added padding to match hlsl vector packing
*/
struct PointLight
{
	// Vector1
	DirectX::XMFLOAT3	DiffuseColor;
	float				InnerRadius;
	// Vector 2
	DirectX::XMFLOAT3	SpecularColor;
	float				OuterRadius;
	// Vector 3
	float				LinearFalloff;
	float				ExponentialFalloff;
	float				_pad0;
	float				_pad1;
};