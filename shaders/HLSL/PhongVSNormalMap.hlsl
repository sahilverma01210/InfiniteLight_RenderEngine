struct VSOutput
{
    float3 viewPos : POSITION;
    float3 normal : NORMAL;
    float3 tan : TANGENT;
    float3 bitan : BITANGENT;
    float2 uv : TEXCOORD;
    float4 pos : SV_POSITION;
};

cbuffer Transform : register(b0)
{
    matrix modelView;
    matrix modelViewProj;
};

VSOutput VSMain(float3 viewPos : POSITION, float3 n : NORMAL, float3 tan : TANGENT, float3 bitan : BITANGENT, float2 uv : TEXCOORD)
{
    VSOutput vso;
    vso.viewPos = (float3) mul(float4(viewPos, 1.0f), modelView);
    vso.normal = mul(n, (float3x3) modelView);
    vso.tan = mul(tan, (float3x3) modelView);
    vso.bitan = mul(bitan, (float3x3) modelView);
    vso.pos = mul(float4(viewPos, 1.0f), modelViewProj);
    vso.uv = uv;
    return vso;
}