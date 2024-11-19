struct PSInput
{
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 position : SV_POSITION;
};

cbuffer Transform : register(b0)
{
    matrix modelView;
    matrix modelViewProj;
};

PSInput VSMain(float4 position : POSITION, float3 normal : Normal, float2 uv : TEXCOORD)
{
    PSInput result;

    result.worldPos = (float3) mul(position, modelViewProj);
    result.normal = mul(normal, (float3x3) modelView);
    result.position = mul(position, modelViewProj);
    result.uv = uv;

    return result;
}