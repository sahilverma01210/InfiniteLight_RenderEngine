#include "Common.hlsli"
#include "Tonemapping.hlsli"

#define BLOCK_SIZE 16

struct ToneMapConstants
{
    int inputTexIdx;
    int outputTexIdx;
};

ConstantBuffer<ToneMapConstants> toneMappingCB : register(b1);

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
    RWTexture2D<float4> inTex = ResourceDescriptorHeap[toneMappingCB.inputTexIdx];
    RWTexture2D<float4> outTex = ResourceDescriptorHeap[toneMappingCB.outputTexIdx];
    
    float4 color = inTex[input.DispatchThreadId.xy];
    
    outTex[input.DispatchThreadId.xy] = float4(LinearToneMapping(color.xyz), 1.0f);
}