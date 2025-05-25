#include "Common.hlsli"
#include "Scene.hlsli"

struct SkyboxCB
{
    int cubeMapTexIdx;
};

float4 main(float3 worldPos : Position) : SV_TARGET
{
    ConstantBuffer<SkyboxCB> skyboxCB = ResourceDescriptorHeap[meshConstants.materialIdx];
    TextureCube<float4> tex = ResourceDescriptorHeap[skyboxCB.cubeMapTexIdx];
    return tex.Sample(LinearWrapSampler, worldPos);
}