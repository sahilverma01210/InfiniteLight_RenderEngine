#include "Transform.hlsl"

struct VSOutput
{
    float3 viewPos : POSITION;
    float3 viewNormal : NORMAL;
    float4 pos : SV_POSITION;
};

VSOutput VSMain(float3 pos : POSITION, float3 n : NORMAL)
{
    VSOutput vso;
    vso.viewPos = (float3) mul(float4(pos, 1.0f), modelView);
    vso.viewNormal = mul(n, (float3x3) modelView);
    vso.pos = mul(float4(pos, 1.0f), modelViewProj);
    return vso;
}