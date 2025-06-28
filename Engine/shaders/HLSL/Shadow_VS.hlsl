#include "Lighting.hlsli"
#include "Scene.hlsli"

struct ShadowIndices
{
    uint currentLightIndex;
    uint currentfaceIndex;
};

ConstantBuffer<ShadowIndices> shadowIndices : register(b3);

float4 main(float3 pos : Position) : SV_Position
{
    StructuredBuffer<Light> light = ResourceDescriptorHeap[frameData.lightDataIdx];
    
    float4x4 meshViewProj = mul(mul(GetMeshMat(), Get360ViewMatrix(shadowIndices.currentfaceIndex, light[shadowIndices.currentLightIndex].pos)), Get360ProjectionMatrix());
    return mul(float4(pos, 1.0f), meshViewProj);
}