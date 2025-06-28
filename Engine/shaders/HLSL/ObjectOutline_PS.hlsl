#include "Common.hlsli"
#include "Scene.hlsli"

struct InputData
{
    int inputFrameIdx;
};

float4 main(float2 uv : Texcoord) : SV_Target
{
    ConstantBuffer<InputData> inputData = ResourceDescriptorHeap[meshConstants.materialIdx];
    Texture2D<float4> tex = ResourceDescriptorHeap[inputData.inputFrameIdx];
    
    return tex.Sample(LinearMirrorSampler, uv);
}