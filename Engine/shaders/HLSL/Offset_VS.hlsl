#include "Transform.hlsl"

cbuffer Offset : register(b2)
{
    float offset;
};

float4 main(float3 pos : Position, float3 n : Normal) : SV_Position
{
    return mul(float4(pos + n * offset, 1.0f), modelViewProj);
}