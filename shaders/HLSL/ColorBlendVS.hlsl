struct VSOutput
{
    float4 color : COLOR;
    float4 position : SV_POSITION;
};

cbuffer Transform
{
    matrix transform;
};

VSOutput VSMain(float4 position : POSITION, float4 color : COLOR)
{
    VSOutput result;

    result.position = mul(position, transform);
    result.color = color;

    return result;
}