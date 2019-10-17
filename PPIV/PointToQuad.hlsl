struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
    float4 Tan : TANGENT;
    float4 Binorm : BINORMAL;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
};

VS_OUTPUT main( VS_INPUT input ) 
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.Pos = input.Pos;
	return output;
}