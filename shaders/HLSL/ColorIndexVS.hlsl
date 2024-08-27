struct VSOutput
{
    float4 position : SV_POSITION;
};

cbuffer Transform
{
    matrix transform;
};

VSOutput VSMain(float4 position : POSITION)
{
    VSOutput result;

    result.position = mul(position, transform);

    return result;
}