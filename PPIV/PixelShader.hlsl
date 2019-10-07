
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
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

//float4 main(PS_INPUT input) : SV_TARGET
//{
//	return txDiffuse.Sample(samLinear, input.Tex);
//}


float4 main(PS_INPUT input) : SV_TARGET
{
	float4 finalColor = (float4)0;
	finalColor = 0;		
	//do NdotL lighting for 2 lights
	for (int i = 0; i < 2; i++)
	{
		finalColor += saturate(dot((float3)vLightDir[i],input.Norm) * vLightColor[i]);
	}
	finalColor *= txDiffuse.Sample(samLinear, input.Tex);
	finalColor.a = 1;
	return finalColor;
}

//float4 main(PS_INPUT input) : SV_TARGET
//{
//	float4 finalColor = 0;
//	finalColor *= txDiffuse.Sample(samLinear, input.Tex);
//	finalColor.a = 1;
//	return finalColor;
//}


//--------------------------------------------------------------------------------------
// PSSolid - render a solid color
//--------------------------------------------------------------------------------------
//float4 PSSolid(PS_INPUT input) : SV_Target
//{
//	return vOutputColor;
//}