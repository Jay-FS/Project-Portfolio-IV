
Texture2D txDiffuse : register(t0);
Texture2D txAO : register(t1);
Texture2D txNM : register(t2);
SamplerState samLinear : register(s0);

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
    float3 worldPos : POSITION;
};

cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 vLightDir[2];
	float4 vLightColor[2];
	float4 vOutputColor;
}

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 finalColor = (float4)0;
    float4 lightRadius = (float4)0;
    float4 attenuation = (float4)0;
    lightRadius = 1;
	finalColor = 0;		
	//do NdotL lighting for 2 lights
    finalColor += saturate(dot((float3) vLightDir[0], input.Norm) * vLightColor[0]);
    finalColor += saturate(dot((float3) vLightDir[1], input.Norm) * vLightColor[1]);

    //attenuation = 1.0f - saturate(length(((float3) vLightDir[1]) - input.worldPos) / lightRadius);
    //finalColor += saturate(dot((float3) vLightDir[1], input.Norm)) * vLightColor[1] * attenuation;
    
    

	finalColor *= txDiffuse.Sample(samLinear, input.Tex);
    float4 AO = txAO.Sample(samLinear, input.Tex);
    float4 NM = txNM.Sample(samLinear, input.Tex);
    finalColor.a = 1; //NM.a;
	return finalColor;
}

//SAMPLE PIXEL SHADER

////Pixel Shader Performing multi)texturing with detail texture on a second uv channel
//texture2D baseTexture : register(t0); // first texture
//texture2D detailTexture : register(t1); // second texture
//SamplerState filters[2] : register(s0); // filter 0 using clamp, filter 1 using wrap
//float4 main(float2 baseUV : TEXCOORD0, float2 detailUV : TEXCOORD1, float4 modulate : COLOR) : SV_TARGET
//{
//    float4 baseColor = baseTexture.Sample(filters[0], baseUV) * modulate; // get base color
//    float4 detailColor = detailTexture.Sample(filters[1], detailUV); // get detail effect
//    float4 finalColor = float4(lerp(baseColor.rgb, detailColor.rgb, detailColor.a), baseColor.a);
//    return finalColor; // return a transition based on the detail alpha
//}


//--------------------------------------------------------------------------------------
// PSSolid - render a solid color
//--------------------------------------------------------------------------------------
float4 PSSolid(PS_INPUT input) : SV_Target
{
	return vOutputColor;
}