#include "Common.hlsli"
#include "Scene.hlsli"

float4 main(float3 pos : Position) : SV_Position
{
    float4x4 meshViewProj = mul(mul(GetMeshMat(), GetViewMat()), GetProjectionMat());
    
    return mul(float4(pos, 1.0f), meshViewProj);
}