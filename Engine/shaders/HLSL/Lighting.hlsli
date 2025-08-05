#include "Common.hlsli"
#include "BRDF.hlsli"

static const float gamma = 2.2;

static float3 LinearToSRGB(float3 linearRGB)
{
    return pow(linearRGB, 1.0f / gamma);
}

float DoAttenuation(float distance, float range)
{
    float att = saturate(1.0f - (distance * distance / (range * range)));
    return att * att;
}

float GetLightAttenuation(Light light, float3 P, out float3 L)
{
    L = normalize(light.viewPos - P);
    float attenuation = 1.0f;
    float distance = length(light.viewPos - P);
    float range = light.range; // Example range, should be set according to light config
    attenuation = DoAttenuation(distance, range);
    return attenuation;
}

///Shadows

float CalcShadowFactor_PCF3x3(SamplerComparisonState shadowSampler,
	Texture2D<float> shadowMap, float3 uvd, int shadowMapSize)
{
    if (uvd.z > 1.0f)
        return 1.0;

    float depth = uvd.z;
    const float dx = 1.0f / shadowMapSize;
    float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };

    float percentLit = 0.0f;
	[unroll(9)] 
    for (int i = 0; i < 9; ++i)
    {
        percentLit += shadowMap.SampleCmpLevelZero(shadowSampler, uvd.xy + offsets[i], depth);
    }
    percentLit /= 9.0f;
    return percentLit;
}

float GetShadowMapFactor(Light light, float3 viewPosition, SamplerComparisonState ShadowWrapSampler)
{
    float shadowFactor = 1.0f;
    
    float3 lightToPixelWS = mul(float4(viewPosition - light.viewPos, 0.0f), GetInverseViewMat()).xyz;
    uint cubeFaceIndex = GetCubeFaceIndex(lightToPixelWS);
    float4 worldPosition = mul(float4(viewPosition, 1.0f), GetInverseViewMat());
    worldPosition /= worldPosition.w;
    float4x4 lightViewProjection = mul(Get360ViewMatrix(cubeFaceIndex, light.position), Get360ProjectionMatrix());
    float4 shadowMapPosition = mul(worldPosition, lightViewProjection);
    float3 UVD = shadowMapPosition.xyz / shadowMapPosition.w;
    UVD.xy = 0.5 * UVD.xy + 0.5;
    UVD.y = 1.0 - UVD.y;
    Texture2D<float> shadowMap = ResourceDescriptorHeap[NonUniformResourceIndex(light.shadowDepthIdx + cubeFaceIndex)];
    shadowFactor = CalcShadowFactor_PCF3x3(ShadowWrapSampler, shadowMap, UVD, 512);
    
    return shadowFactor;
}

float3 DoLight_Default(Light light, BrdfData brdfData, float3 P, float3 N, float3 V, float2 uv, SamplerComparisonState ShadowWrapSampler)
{
    float3 L;
    float attenuation = GetLightAttenuation(light, P, L);
    if (attenuation <= 0.0f)
        return 0.0f;
    
    attenuation *= GetShadowMapFactor(light, P, ShadowWrapSampler);
    if (attenuation <= 0.0f)
        return 0.0f;

    float NdotL = saturate(dot(N, L));
    if (NdotL == 0.0f)
        return 0.0f;

    float3 brdf = DefaultBRDF(L, V, N, brdfData.Diffuse, brdfData.Specular, brdfData.Roughness);
    return brdf * light.diffuseColor * light.diffuseIntensity * attenuation * NdotL;
}