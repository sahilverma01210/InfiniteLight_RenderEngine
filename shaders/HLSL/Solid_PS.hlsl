cbuffer CBuf : register(b2)
{
    float3 materialColor[1];
};

float4 main() : SV_Target
{
    return float4(materialColor[0], 1.0f);;
}