struct PSInput
{
    float4 position : SV_POSITION;
};

cbuffer Transform
{
    matrix transform;
};

PSInput VSMain(float4 position : POSITION)
{
	PSInput result;

    result.position = mul(position, transform);

    return result;
}