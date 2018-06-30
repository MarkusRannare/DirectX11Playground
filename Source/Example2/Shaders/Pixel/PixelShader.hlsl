// @todo: Either combine vertex/pixelshader in one file (ie, materials), or some other solution to not 
// need to redefine VertexOut in two files
struct VertexOut
{
	float4 PosH		: SV_POSITION;
	float4 Color	: COLOR;
};

float4 main( VertexOut Pin ) : SV_TARGET
{
	return Pin.Color;
}