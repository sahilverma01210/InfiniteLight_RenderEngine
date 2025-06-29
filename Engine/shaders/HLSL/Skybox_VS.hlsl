#include "Common.hlsli"
#include "Scene.hlsli"

struct VSOut
{
    float3 worldPos : Position;
    float4 pos : SV_Position;
};

VSOut main(float3 pos : Position)
{
    float4x4 meshViewProj = mul(mul(GetMeshMat(), GetViewMat()), GetProjectionMat());
    
    VSOut vso;
    vso.worldPos = pos;
    vso.pos = mul(float4(pos, 0.0f), meshViewProj);
    // make sure that the depth after w divide will be 1.0 (so that the z-buffering will work)
    vso.pos.z = vso.pos.w;
    return vso;
}