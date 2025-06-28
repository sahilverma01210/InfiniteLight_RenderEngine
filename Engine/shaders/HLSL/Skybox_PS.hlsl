#include "Common.hlsli"
#include "Scene.hlsli"

float4 main(float3 worldPos : Position) : SV_TARGET
{
    TextureCube tex = ResourceDescriptorHeap[frameData.envMapIdx];
    return tex.Sample(LinearWrapSampler, worldPos);
}