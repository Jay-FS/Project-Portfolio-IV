

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
}

cbuffer TimeBuffer : register(b1)
{
    float time;
    float3 padding;
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
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.Pos = mul(input.Pos, World);
    output.Pos.y = sin(mul(input.Pos.y, time));
    output.worldPos = output.Pos.xyz;
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.Norm = mul(float4(input.Norm, 0.0f), World).xyz;
    output.Tex = input.Pos.xyz;

    return output;
}