cbuffer CubeColors : register(b2)
{
    float4 colors[1];
};

float4 PSMain() : SV_TARGET
{
    return colors[0];
}