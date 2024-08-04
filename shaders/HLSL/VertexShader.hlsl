struct VSOutput
{
    float2 uv : TEXCOORD;
    float4 position : SV_POSITION;
};

cbuffer Transform
{
    matrix transform;
};

VSOutput VSMain(float4 position : POSITION, float2 uv : TEXCOORD)
{
    VSOutput result;

    result.position = mul(position, transform);
    result.uv = uv;

    return result;
}