struct VSOutput
{
    float4 position : SV_POSITION;
};

cbuffer Transform : register(b0)
{
    matrix modelView;
    matrix modelViewProj;
};

VSOutput VSMain(float4 position : POSITION) : SV_Position
{
    VSOutput result;

    result.position = mul(position, modelViewProj);

    return result;
}