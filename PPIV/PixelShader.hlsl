
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
    float4 Tan : TANGENT;
    float4 Binorm : BINORMAL;

};

cbuffer LightBuffer : register(b0)
{
    float4 lightDir[2];
    float4 lightColor[4];
    float4 lightPos;
    float lightRadius;
    float3 padding0;
    float4 coneDir;
    float coneSize;
    float coneRatio;
    float innerConeRatio;
    float outerConeRatio;
}

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 finalColor = (float4)0; // Result
    float attenuation = 0;
    int numLights = 2;
	finalColor = 0;		
    attenuation = 0;
    
    // NORMAL MAPPING
    float4 NM = txNM.Sample(samLinear, input.Tex);
    
    NM = (2.0f * NM) - 1.0f;
    // making the tan orthogonal to the normal
    input.Tan.xyz = normalize(input.Tan.xyz);
    input.Binorm.xyz = normalize(input.Binorm.xyz);
    input.Norm.xyz = normalize(input.Norm.xyz);
    //float3 newTan = normalize(input.Tan.xyz - (dot(input.Tan.xyz, input.Norm.xyz) * input.Norm.xyz));
    //float3x3 texSpace = float3x3(newTan.xyz, input.Binorm.xyz, input.Norm.xyz);
    float3x3 texSpace = float3x3(input.Tan.xyz, input.Binorm.xyz, input.Norm.xyz);
    float3 newNorm = normalize(mul(NM.xyz, texSpace));

    //LIGHTING

    //Directional light
    float4 DirectionalLight = saturate(dot(lightDir[0].xyz, newNorm)) * lightColor[0];
        

    //Point Light
    float3 lightPosition = normalize(lightPos.xyz - input.worldPos);
    float4 lightRatio1 = saturate(dot(lightPosition, newNorm));
        //Fall off by radius
    attenuation = 1.0 - saturate(length(lightPos.xyz - input.worldPos) / lightRadius);
    float4 PointLight = lightRatio1 * lightColor[3] * attenuation;


        //Spot Light
    float3 newDir = normalize(lightPos.xyz - input.worldPos);
    float surfaceRatio = saturate(dot(-newDir, normalize(coneDir.xyz)));
        //Fall off by Cone Edge
    attenuation = (1.0 - saturate((innerConeRatio - surfaceRatio) / (innerConeRatio - outerConeRatio)));
    attenuation *= (1.0 - saturate(length(lightPos.xyz - input.worldPos) / lightRadius));
    float spotFactor = (surfaceRatio > outerConeRatio) ? 1 : 0;
    float lightRatio = saturate(dot(newDir, newNorm));
    float4 SpotLight = spotFactor * lightRatio * lightColor[2] * attenuation;


    float4 ambient = txDiffuse.Sample(samLinear, input.Tex) * 0.5f;

    //finalColor += DirectionalLight;
    //finalColor += PointLight;
    finalColor += SpotLight;
    //finalColor += ambient;

        //FINAL TEXTURE AND AO
    float4 AO = txAO.Sample(samLinear, input.Tex);
    finalColor *= txDiffuse.Sample(samLinear, input.Tex);
    finalColor *= AO;

    return saturate(finalColor);
}

