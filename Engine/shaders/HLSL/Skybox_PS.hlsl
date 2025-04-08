#include "CommonResources.hlsli"

struct SkyboxCB
{
    int cubeMapTexIdx;
};

SamplerState samplerState : register(s0);

float4 main(float3 worldPos : Position) : SV_TARGET
{
    ConstantBuffer<SkyboxCB> skyboxCB = ResourceDescriptorHeap[meshConstants.materialIdx];
    TextureCube<float4> tex = ResourceDescriptorHeap[skyboxCB.cubeMapTexIdx];
    return tex.Sample(samplerState, worldPos);
}