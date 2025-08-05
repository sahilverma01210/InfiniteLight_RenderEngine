#include "Common.hlsli"

struct Color
{
    float red;
    float green;
    float blue;
};

ConstantBuffer<Color> color : register(b2);

float4 main() : SV_Target
{
    return float4(color.red, color.green, color.blue, 1.0f);
}