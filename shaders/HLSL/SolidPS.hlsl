cbuffer CBuf : register(b2)
{
    float4 colors[1];
};

float4 main() : SV_Target
{
    return colors[0];
}