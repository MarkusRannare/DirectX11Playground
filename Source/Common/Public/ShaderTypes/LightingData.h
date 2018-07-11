#pragma once

// @todo: Send this value over to shaders so we don't need to manually mirror this
#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 32
#include "DirectionalLight.h"
#include "PointLight.h"
#include <DirectXMath.h>

// Mirrored in shaders, if you update this, make sure to update Lighting.hlsl
// Make sure it's padded properly as it's memcopied across
struct ShaderLighting
{
	DirectionalLight	DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
	PointLight			PointLights[MAX_POINT_LIGHTS];
	DirectX::XMFLOAT3	AmbientLightColor;
	int					NumDirectionalLights;
	DirectX::XMFLOAT3	EyeLocaiton;
	int					NumPointLights;
};