#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 32

struct DirectionalLight
{
	float3 DiffuseColor;
	float3 SpecularColor;
	float3 Direction;
};

struct PointLight
{
	float3		wsLocation;
	float		LinearFalloff;
	// Vector2
	float3		DiffuseColor;
	float		ExponentialFalloff;
	// Vector 3
	float3		SpecularColor;
	float		OuterRadius;
	// Vector 4
	float		Intensity;
	// Add explcit padding so that we be sure to know how it's padded in cbuffer arrays
	float		_pad0;
	float		_pad1;
	float		_pad2;
};

cbuffer Lighting
{
	DirectionalLight	DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
	PointLight			PointLights[MAX_POINT_LIGHTS];
	float3				AmbientLightColor;
	int					NumDirectionalLights;
	float3				wsEyeLocation;
	int					NumPointLights;
};

float3 Specular(float3 wsLightDir, float3 SpecularColor, float3 wsNormal, float3 wsToEye)
{
	float3 HalfVector = normalize(wsLightDir + wsToEye);
	// @todo: Expose the specular exponent to some material system
	float SpecularFactor = pow(saturate(dot(wsNormal, HalfVector)), 20.0f);

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

			FinalLighting += Diffuse + Specular( -DirectionalLights[i].Direction, DirectionalLights[i].SpecularColor, wsNormal, wsToEye );
		}
	}

	return FinalLighting;
}

float Attenuate( float3 wsVertexPos, PointLight Light )
{
	// @todo: Verify that we don't calculate length twice now in CalculatePointLighting, as length is calculated in the
	// outside for-loop
	float Distance = length( wsVertexPos - Light.wsLocation );
	return /*Light.Intensity*/ 1.0f / ( 1.0f + Light.LinearFalloff * Distance + Light.ExponentialFalloff * Distance * Distance );
}

float3 CalculatePointLighting( float3 wsVertexPos, float3 wsNormal, float3 wsToEye )
{
	float3 FinalLighting = float3(0, 0, 0);
	for (int i = 0; i < NumPointLights; ++i)
	{
		float3 LightDir = PointLights[i].wsLocation - wsVertexPos;
		const float LightDistance = length(LightDir);

		// Don't do any more calculations for this light if we are outside the radius of the light
		if( LightDistance > PointLights[i].OuterRadius )
		{
			continue;
		}

		// Can this cause a division by zero and cause artifacts?
		LightDir /= LightDistance;
		const float DiffuseFactor = dot( wsNormal, LightDir );

		// Only do the calculations if the diffuse is positive, if negiative it shouldn't darken the pixel
		[flatten]
		if( DiffuseFactor > 0.0f )
		{
			float3 Diffuse = DiffuseFactor * PointLights[i].DiffuseColor;
			float3 Spec = Specular(LightDir, PointLights[i].SpecularColor, wsNormal, wsToEye);

			FinalLighting += ( Diffuse + Spec ) * Attenuate( wsVertexPos, PointLights[i] );
		}
	}
	return FinalLighting;
}

/*float3 CalculateSpotLighting( float3 wsVertexPos, float3 wsNormal, float3 wsToEye )
{
}*/

float3 CalculateLighting( float3 wsPos, float3 wsNormal )
{
	const float3 wsToEye = normalize(wsEyeLocation - wsPos);

	const float3 DirectionalLighting = CalculateDirectionalLighting(wsNormal, wsToEye);
	const float3 PointLighting = CalculatePointLighting( wsPos, wsNormal, wsToEye );
	// const float3 SpotLighting = CalculateSpotLighting( wsPos, wsNormal, wsToEye );
	// @todo: Calculate lighting from spot lights

	const float3 FinalLighting = DirectionalLighting + PointLighting + AmbientLightColor;
	return clamp(FinalLighting, 0.0f, 1.0f);
}