
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
    float4 lightColor[2];
    float coneSize;
    float coneDir;
    float coneRange;
    float coneRatio;
}

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 finalColor = (float4)0;
    int numLights = 2;
	finalColor = 0;		

    
    // NORMAL MAPPING
    float4 NM = txNM.Sample(samLinear, input.Tex);

    NM = (2.0f * NM) - 1.0f;
    // making the tan orthogonal to the normal
    float3 newTan = normalize((float3) input.Tan - (dot((float3) input.Tan, input.Norm) * input.Norm));
    float3x3 texSpace = float3x3(newTan, (float3) input.Binorm, input.Norm);
    float3 newNorm = saturate(mul((float1x3) NM, texSpace));

    //LIGHTING

    //Fall off by radius
    //attenuation = 1.0 - saturate(length(lightDir - input.worldPos) / lightRadius);

    //Fall off by Cone Edge
    //attenuation = 1.0 - saturate(innerConeRatio - surfaceRatio) / innerConeRatio - outerConeRatio));
    
    //Make Attenuation quadratic
    //attenuation *= attenuation;
        
    for (int i = 0; i < numLights; i++)
    {
	    //Directional light
        float3 newDir = -normalize(-lightDir[i]); // normalize the light direction
        finalColor += saturate(dot( newDir, newNorm)) * lightColor[i]; // * attenuation;
        
        //Point Light
        //float3 lightDirection = normalize(lightDir[i].xyz - input.worldPos);
        //float4 lightRatio = saturate(dot(lightDirection, newNorm));
        //finalColor = lightRatio * lightColor[i];

        //Spot Light
        //float3 newDir = normalize(lightDir[i].xyz - input.worldPos);
        //float surfaceRatio = saturate(dot(-newDir, coneDir));
        //float spotFactor = (surfaceRatio > coneRatio) ? 1 : 0;
        //float lightRatio = saturate(dot(newDir, newNorm));
        //finalColor = spotFactor * lightRatio * lightColor[i];
        
    }
    //FINAL TEXTURE AND AO
    float4 AO = txAO.Sample(samLinear, input.Tex); 
	finalColor *= txDiffuse.Sample(samLinear, input.Tex);
    finalColor *= AO;

	return finalColor;
}

