#include "Transform.hlsl"

struct VSOutput
{
    float4 position : SV_POSITION;
};

VSOutput VSMain(float4 position : POSITION) : SV_Position
{
    VSOutput result;

    result.position = mul(position, modelViewProj);

    return result;
}