#include "ShaderOps.hlsl"
#include "LightVectorData.hlsl"
#include "PointLight.hlsl"

cbuffer ObjectCBuf : register(b2)
{
    float4 materialColor;
    float4 specularColor;
    float specularPower;
};

SamplerState samp;

float4 PSMain(float3 viewFragPos : POSITION, float3 viewNormal : NORMAL) : SV_TARGET
{
	// normalize the mesh normal
    viewNormal = normalize(viewNormal);
	// fragment to light vector data
    const LightVectorData lv = CalculateLightVectorData(viewLightPos, viewFragPos);
	// attenuation
    const float att = Attenuate(attConst, attLin, attQuad, lv.distToL);
	// diffuse
    const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, viewNormal);
    // specular
    const float3 specular = Speculate(
        specularColor.rgb, 1.0f, viewNormal,
        lv.vToL, viewFragPos, att, specularPower
    );
	// final color
    return float4(saturate((diffuse + ambient) * materialColor.rgb + specular), 1.0f);
}