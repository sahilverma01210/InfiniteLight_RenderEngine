Texture2D tex;
SamplerState samp;

cbuffer CubeColors
{
    float4 colors[6];
};

float4 PSMain(float2 uv : TEXCOORD, uint primitiveID : SV_PrimitiveID) : SV_TARGET
{
    //return tex.Sample(samp, uv);
    return colors[primitiveID >> 1];
}