// @todo: Prettyfy this so that we don't have to specify so arbitrary paths
#include "../../../Common/Shaders/Lighting.hlsl"

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

float4 main( VertexOut Pin ) : SV_TARGET
{
	const float3 Light = saturate( CalculateLighting( Pin.PosWS, Pin.NormalWS ) );

	return float4( Pin.Color.xyz * Light, 1.0f );
}