cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
};

struct VertexIn
{
	float3 Pos		: POSITION;
    float4 Color	: COLOR;
};

// @todo: Either combine vertex/pixelshader in one file (ie, materials), or some other solution to not 
// need to redefine VertexOut in two files
struct VertexOut
{
	float4 PosH		: SV_POSITION;
	float4 Color	: COLOR;
};

VertexOut main( VertexIn Vin )
{
    VertexOut Out;

    Out.PosH = mul(float4(Vin.Pos, 1.0f), gWorldViewProj);
	Out.Color = float4( Vin.Color.rgb, 1.0f );

	return Out;
}