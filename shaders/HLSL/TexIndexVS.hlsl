struct PSInput
{
    float2 uv : TEXCOORD;
    float4 position : SV_POSITION;
};

cbuffer Transform
{
    matrix transform;
};

PSInput VSMain(float4 position : POSITION, float2 uv : TEXCOORD)
{
    PSInput result;

    result.position = mul(position, transform);
    result.uv = uv;

    return result;
}