#include "CommonResources.hlsli"

float4 main(float3 pos : Position) : SV_Position
{
    return mul(float4(pos, 1.0f), meshTransforms.meshViewProj);
}