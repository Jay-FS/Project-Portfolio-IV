
cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
}


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
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
    float3 worldPos : POSITION;
    float4 Tan : TANGENT;
    float4 Binorm : BINORMAL;
};
VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(input.Pos, World);
    output.worldPos = output.Pos;
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	output.Norm = mul(float4(input.Norm, 1), World).xyz;
	output.Tex = input.Tex;
    output.Tan = mul(float4(input.Tan.xyz * input.Tan.w, 0.0f), World);
    output.Binorm = mul(float4(cross(input.Norm.xyz, input.Tan.xyz), 0.0f), World);
	return output;
}