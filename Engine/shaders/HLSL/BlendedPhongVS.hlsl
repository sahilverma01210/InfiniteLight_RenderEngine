struct VSOutput
{
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float3 color : Color;
    float4 pos : SV_POSITION;
};

cbuffer Transform : register(b0)
{
    matrix modelView;
    matrix modelViewProj;
};

VSOutput VSMain(float3 pos : POSITION, float3 n : NORMAL, float3 color : COLOR)
{
    VSOutput vso;
    vso.worldPos = (float3) mul(float4(pos, 1.0f), modelView);
    vso.normal = mul(n, (float3x3) modelView);
    vso.pos = mul(float4(pos, 1.0f), modelViewProj);
    vso.color = color;
    return vso;
}