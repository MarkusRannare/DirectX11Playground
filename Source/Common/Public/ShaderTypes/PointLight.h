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
	DirectX::XMFLOAT3	Location;
	float				LinearFalloff;
	// Vector2
	DirectX::XMFLOAT3	DiffuseColor;
	float				ExponentialFalloff;
	// Vector 3
	DirectX::XMFLOAT3	SpecularColor;
	float				OuterRadius;
	// Vector 4
	float				Intensity;
	// @todo: Add Inner radius
	float				_pad0;
	float				_pad1;
	float				_pad2;
};