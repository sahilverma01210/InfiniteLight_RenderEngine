cbuffer CubeColors
{
    float4 colors[6];
};

float4 PSMain(uint primitiveID : SV_PrimitiveID) : SV_TARGET
{
    return colors[primitiveID >> 1];
}