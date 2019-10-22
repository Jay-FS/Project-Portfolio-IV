TextureCube txDiffuse : register(t0);
SamplerState samLinear : register(s0);


struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float3 Tex : TEXCOORD0;
    float3 worldPos : POSITION;
    float4 Tan : TANGENT;  
    float4 Binorm : BINORMAL;

};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 finalColor = (float4) 0;
    finalColor = txDiffuse.Sample(samLinear, input.Tex.xyz) * 0.9f;
    //float3 lerped = lerp(); // lerp with world position
    return finalColor ;

}