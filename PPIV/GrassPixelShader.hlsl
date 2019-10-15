Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
    float3 worldPos : POSITION;
};

cbuffer LightBuffer : register(b0)
{
    float4 lightDir[2];
    float4 lightColor[2];
    float4 lightRadius;
    float coneSize;
    float coneDir;
    float coneRatio;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 finalColor = (float4) 0;
    finalColor = 0;

    finalColor += saturate(dot((float3) lightDir[0], input.Norm)) * lightColor[0];
    finalColor *= txDiffuse.Sample(samLinear, input.Tex);


        //Point Light
    //float3 lightDirection = normalize(lightDir[0].xyz - input.worldPos);
    //float4 lightRatio = saturate(dot(lightDirection, input.Norm));
    //    //attenuation = 1.0 - saturate(length(lightDir[i].xyz - input.worldPos) / lightRadius);
    //finalColor = lightRatio * lightColor[0]; // * attenuation;
	return finalColor;
}