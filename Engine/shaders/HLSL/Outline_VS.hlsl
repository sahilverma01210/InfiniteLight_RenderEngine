#include "Common.hlsli"

struct VSOut
{
    float2 uv : Texcoord;
    float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 norm : Normal, float2 texUV : Texcoord, float3 tan : Tangent, float3 bitan : Bitangent)
{
    VSOut vso;
    vso.pos = float4(pos, 1.0f);
    vso.uv = float2((pos.x + 1) / 2.0f, -(pos.y - 1) / 2.0f);
    return vso;
}