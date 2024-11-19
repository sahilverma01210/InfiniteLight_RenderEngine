cbuffer CubeColors : register(b1)
{
    float4 colors[8];
};

float4 PSMain(uint primitiveID : SV_PrimitiveID) : SV_TARGET
{
    //return tex.Sample(samp, uv);
    return colors[(primitiveID / 2) % 8];
}