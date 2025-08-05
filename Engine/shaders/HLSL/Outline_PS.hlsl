#include "Common.hlsli"

struct InputData
{
    int inputFrameIdx;
};

ConstantBuffer<InputData> inputData : register(b1);

float4 main(float2 uv : Texcoord) : SV_Target
{
    Texture2D<float4> tex = ResourceDescriptorHeap[inputData.inputFrameIdx];
    
    return tex.Sample(LinearMirrorSampler, uv);
}