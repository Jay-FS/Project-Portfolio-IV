TextureCube txDiffuse : register(t0);
SamplerState samLinear : register(s0);


cbuffer TimeBuffer : register(b0)
{
    float time;
    float3 padding;
}

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float3 Tex : TEXCOORD0;
    float3 worldPos : POSITION0;
    float4 Tan : TANGENT;
    float4 Binorm : BINORMAL;

};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 finalColor = (float4) 0;
    // worldPos - camPos
    float3 vToCam = normalize(input.worldPos.xyz - padding.xyz); // padding = cam pos
    //reflect ^ vector(this, worldNorm)
    float3 reflection = normalize(reflect(vToCam, normalize(input.Norm)));
    // sample returned reflect used as uv coord
    finalColor = txDiffuse.Sample(samLinear, reflection) * 0.8f;
    //float3 lerped = lerp(); // lerp with world position
    return finalColor;

}