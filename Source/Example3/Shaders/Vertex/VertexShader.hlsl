cbuffer cbPerObject
{
	float4x4 gWorldViewProj;
};

struct VertexIn
{
	float3 Pos		: POSITION;
    float4 Color	: COLOR;
	float3 Normal	: NORMAL;
};

// @todo: Either combine vertex/pixelshader in one file (ie, materials), or some other solution to not 
// need to redefine VertexOut in two files
struct VertexOut
{
	float4 PosHS		: SV_POSITION;
	float3 PosWS		: POSITION0;
	float4 Color		: COLOR;
	float3 NormalWS		: NORMAL;
};

VertexOut main( VertexIn Vin )
{
    VertexOut Out;

	// @todo: Transform position and normal so we can rotate objects
	Out.PosWS = Vin.Pos;
	Out.NormalWS = Vin.Normal;
	Out.Color = Vin.Color;
    Out.PosHS = mul(float4(Vin.Pos, 1.0f), gWorldViewProj);

	return Out;
}