#include "Lighting.hlsli"

#define BLOCK_SIZE 16

struct MaterialHandle
{
    int lightResourceHandlesIdx;
};

struct LightResourceHandles
{
    int renderTargetIdx;
    int frameConstIdx;
    int lightConstIdx;
    int diffTexIdx;
    int normTexIdx;
    int metallicRoughTexIdx;
    int depthTexIdx;
};

struct FrameCBuffer
{
    float2 renderResolution;
};

ConstantBuffer<MaterialHandle> matCB : register(b1);

struct CSInput
{
    uint3 GroupId : SV_GroupID;
    uint3 GroupThreadId : SV_GroupThreadID;
    uint3 DispatchThreadId : SV_DispatchThreadID;
    uint GroupIndex : SV_GroupIndex;
};

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void main(CSInput input)
{
    ConstantBuffer<LightResourceHandles> lightResourceHandles = ResourceDescriptorHeap[matCB.lightResourceHandlesIdx];    
    
    ConstantBuffer<FrameCBuffer> frameCB = ResourceDescriptorHeap[lightResourceHandles.frameConstIdx];
    Texture2D normalRT = ResourceDescriptorHeap[lightResourceHandles.normTexIdx];
    Texture2D diffuseRT = ResourceDescriptorHeap[lightResourceHandles.diffTexIdx];
    Texture2D<float> depthTexture = ResourceDescriptorHeap[lightResourceHandles.depthTexIdx];
    
    StructuredBuffer<Light> pointLightCB = ResourceDescriptorHeap[lightResourceHandles.lightConstIdx];

    float2 uv = ((float2) input.DispatchThreadId.xy + 0.5f) * 1.0f / (frameCB.renderResolution);

    // viewNormal, metallic, shadingExtension
    float3 viewNormal;
    float metallic;
    uint shadingExtension;
    float4 normalRTData = normalRT.Sample(LinearWrapSampler, uv);
    DecodeGBufferNormalRT(normalRTData, viewNormal, metallic, shadingExtension);
    
    // depth
    float depth = depthTexture.Sample(LinearWrapSampler, uv);

    // viewPosition
    float3 viewPosition = GetViewPosition(uv, depth);
    float3 V = normalize(float3(0.0f, 0.0f, 0.0f) - viewPosition);

    // albedo, roughness
    float4 albedoRoughness = diffuseRT.Sample(LinearWrapSampler, uv);
    float3 albedo = albedoRoughness.rgb;
    float roughness = albedoRoughness.a;
	
    // Reflection
    BrdfData brdfData = GetBrdfData(albedo, metallic, roughness);
    float3 directLighting = 0.0f;
    
    // Shadow
    Light light = pointLightCB[0];
    directLighting += DoLight_Default(light, brdfData, viewPosition, viewNormal, V, uv, ShadowWrapSampler);
    
    // Ambient contribution
    directLighting += light.ambient * albedo;
    
    // Tonemapping
    directLighting = LinearToSRGB(directLighting);
	
    RWTexture2D<float4> outTex = ResourceDescriptorHeap[lightResourceHandles.renderTargetIdx];
    outTex[input.DispatchThreadId.xy] = float4(directLighting, 1.0f);
}