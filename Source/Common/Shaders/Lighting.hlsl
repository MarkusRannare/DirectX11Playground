#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 64

struct DirectionalLight
{
	float3 DiffuseColor;
	float3 SpecularColor;
	float3 Direction;
};

struct PointLight
{
	float3	DiffuseColor;
	float	InnerRadius;
	float3	SpecularColor;
	float	OuterRadius;
	float	LinearFalloff;
	float	ExponentialFalloff;
};

cbuffer Lighting
{
	DirectionalLight	DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
	//PointLight			PointLights[MAX_POINT_LIGHTS];
	float3				AmbientLightColor;
	int					NumDirectionalLights;
	float3				wsEyeLocation;
	int					NumPointLights;
};

float3 Specular(float3 wsLightDir, float3 SpecularColor, float3 wsNormal, float3 wsToEye)
{
	float3 HalfVector = normalize(-wsLightDir + wsToEye);
	// @todo: Expose the specular exponent to some material system
	float SpecularFactor = pow(saturate(dot(wsNormal, HalfVector)), 16.0f);

	return SpecularFactor * SpecularColor;
}

float3 CalculateDirectionalLighting(float3 wsNormal, float3 wsToEye)
{
	// @todo: Do NumDirectionalLights as a define, and select shaders depending on number of 
	// directional lights instead
	float3 FinalLighting = float3(0, 0, 0);
	for (int i = 0; i < NumDirectionalLights; ++i)
	{
		float DiffuseFactor = dot(wsNormal, -DirectionalLights[i].Direction);

		// Only do the calculations if the diffuse is positive, if negiative it shouldn't darken the pixel
		[flatten]
		if( DiffuseFactor > 0.0f )
		{
			float3 Diffuse = DiffuseFactor * DirectionalLights[i].DiffuseColor;

			FinalLighting += Diffuse + Specular( DirectionalLights[i].Direction, DirectionalLights[i].SpecularColor, wsNormal, wsToEye );
		}
	}

	return FinalLighting;
}

float3 CalculatePointLighting(float3 wsPos, float3 wsNormal )
{
	// @todo: implement
	return float3( 0, 0, 0 );
}

float3 CalculateLighting( float3 wsPos, float3 wsNormal )
{
	const float3 ToEye = normalize(wsEyeLocation - wsPos);

	const float3 DirectionalLighting = CalculateDirectionalLighting(wsNormal, ToEye);
	// @todo: Calculate lighting from point lights
	// @todo: Calculate lighting from spot lights

	const float3 FinalLighting = DirectionalLighting + AmbientLightColor;
	return clamp(FinalLighting, 0.0f, 1.0f);
}