#include "CommonResources.hlsli"

struct PostProcessCB
{
    int frameBufferIdx;
};

SamplerState samplerState : register(s0);

float4 main(float2 uv : Texcoord) : SV_Target
{
    ConstantBuffer<PostProcessCB> postProcessCB = ResourceDescriptorHeap[meshConstants.materialIdx];
    Texture2D<float4> tex = ResourceDescriptorHeap[postProcessCB.frameBufferIdx];
    
    return tex.Sample(samplerState, uv);
}