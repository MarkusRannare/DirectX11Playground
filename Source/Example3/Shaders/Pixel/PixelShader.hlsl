// @todo: Either combine vertex/pixelshader in one file (ie, materials), or some other solution to not 
// need to redefine VertexOut in two files

// ws = world space
// os = object space
// ts = tangent space
// es = eye space
// ls = light space

struct VertexOut
{
	float4 PosHS		: SV_POSITION;
	float3 PosWS		: POSITION0;
	float4 Color		: COLOR;
	float3 NormalWS		: NORMAL;
};

struct DirectionalLight
{
	float3 DiffuseColor;
	float3 SpecularColor;
	float3 Direction;
};

#define MAX_DIRECTIONAL_LIGHTS 4

cbuffer Lighting
{
	DirectionalLight DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
	float3 AmbientLightColor;
	int NumDirectionalLights;
	float3 wsEyeLocation;
};

float3 CalculateDirectionalLighting( float3 wsNormal, float3 ToEye )
{
	float3 FinalLighting = float3( 0, 0, 0 );
	for( int i = 0; i < NumDirectionalLights; ++i )
	{
		float DiffuseFactor = dot(wsNormal, -DirectionalLights[i].Direction);

		// Only do the calculations if the diffuse is positive, if negiative it shouldn't darken the
		// pixel
		[flatten]
		if(DiffuseFactor > 0.0f )
		{
			float3 Diffuse = DiffuseFactor * DirectionalLights[i].DiffuseColor;

			// Add specular highlight
			float3 HalfVector = normalize( -DirectionalLights[i].Direction + ToEye);
			float SpecularFactor = pow( saturate( dot( wsNormal, HalfVector ) ), 16.0f );
			float3 Specular = SpecularFactor * DirectionalLights[i].SpecularColor;

			FinalLighting += Diffuse + Specular;
		}
	}
	
	return FinalLighting;
}

float3 CalculateLighting( float3 wsPos, float3 wsNormal )
{
	const float3 ToEye = normalize( wsEyeLocation - wsPos );

	// @todo: Calculate lighting from directional lights
	const float3 DirectionalLighting = CalculateDirectionalLighting( wsNormal, ToEye );
	// @todo: Calculate lighting from spot lights
	// @todo: Calculate lighting from point lights

	const float3 FinalLighting = DirectionalLighting + AmbientLightColor;
	return clamp(FinalLighting, 0.0f, 1.0f );
}

float4 main( VertexOut Pin ) : SV_TARGET
{
	const float3 Light = saturate( CalculateLighting( Pin.PosWS, Pin.NormalWS ) );

	return float4( Pin.Color.xyz * Light, 1.0f );
}