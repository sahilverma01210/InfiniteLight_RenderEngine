#include "Transform.hlsl"

struct VSOutput
{
    float3 viewPos : POSITION;
    float3 viewNormal : NORMAL;
    float3 tan : TANGENT;
    float3 bitan : BITANGENT;
    float2 uv : TEXCOORD;
    float4 pos : SV_POSITION;
};

VSOutput VSMain(float3 pos : POSITION, float3 n : NORMAL, float3 tan : TANGENT, float3 bitan : BITANGENT, float2 uv : TEXCOORD)
{
    VSOutput vso;
    vso.viewPos = (float3) mul(float4(pos, 1.0f), modelView);
    vso.viewNormal = mul(n, (float3x3) modelView);
    vso.tan = mul(tan, (float3x3) modelView);
    vso.bitan = mul(bitan, (float3x3) modelView);
    vso.pos = mul(float4(pos, 1.0f), modelViewProj);
    vso.uv = uv;
    return vso;
}