#include "Common.hlsli"

struct VSOut
{
    float3 worldPos : Position;
    float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 norm : Normal, float2 texUV : Texcoord, float3 tan : Tangent, float3 bitan : Bitangent)
{
    float4x4 viewProj = mul(GetViewMat(), GetProjectionMat());
    
    VSOut vso;
    vso.worldPos = pos;
    vso.pos = mul(float4(pos, 0.0f), viewProj);    
    vso.pos.z = vso.pos.w; // depth after w divide will be 1.0 (so that the z-buffering will work)
    return vso;
}