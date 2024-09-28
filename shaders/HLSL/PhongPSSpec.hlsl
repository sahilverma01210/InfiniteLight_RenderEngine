#include "ShaderOps.hlsl"
#include "LightVectorData.hlsl"
#include "PointLight.hlsl"

cbuffer ObjectCBuf : register(b2)
{
    float specularPowerConst;
    bool hasGloss;
    float specularMapWeight;
};

Texture2D<float4> tex[2] : register(t0);
SamplerState samp;

float4 PSMain(float3 viewFragPos : POSITION, float3 viewNormal : NORMAL, float2 uv : TEXCOORD) : SV_TARGET
{
	// normalize the mesh normal
    viewNormal = normalize(viewNormal);
	// fragment to light vector data
    const LightVectorData lv = CalculateLightVectorData(viewLightPos, viewFragPos);
    // specular parameters
    float specularPower = specularPowerConst;
    const float4 specularSample = tex[1].Sample(samp, uv);
    const float3 specularReflectionColor = specularSample.rgb * specularMapWeight;
    if (hasGloss)
    {
        specularPower = pow(2.0f, specularSample.a * 13.0f);
    }
	// attenuation
    const float att = Attenuate(attConst, attLin, attQuad, lv.distToL);
	// diffuse light
    const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, viewNormal);
    // specular reflected
    const float3 specularReflected = Speculate(
        specularReflectionColor, 1.0f, viewNormal,
        lv.vToL, viewFragPos, att, specularPower
    );
	// final color = attenuate diffuse & ambient by diffuse texture color and add specular reflected
    return float4(saturate((diffuse + ambient) * tex[0].Sample(samp, uv).rgb + specularReflected), 1.0f);
}