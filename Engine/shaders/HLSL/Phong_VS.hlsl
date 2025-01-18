#include "VertexShaderUtils.hlsl"

ConstantBuffer<MeshTransforms> meshTransforms : register(b0, space1);
ConstantBuffer<ShadowTransforms> shadowTransforms : register(b0);

struct VSOut
{
    float3 viewPos : Position;
    float3 viewNormal : Normal;
    float3 viewTan : Tangent;
    float3 viewBitan : Bitangent;
    float2 texUV : Texcoord;
    float4 shadowPos : ShadowPosition;
    float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 n : Normal, float2 texUV : Texcoord, float3 tan : Tangent, float3 bitan : Bitangent)
{
    VSOut vso;
    vso.viewPos = (float3) mul(float4(pos, 1.0f), meshTransforms.meshView);
    vso.viewNormal = mul(n, (float3x3) meshTransforms.meshView);
    vso.viewTan = mul(tan, (float3x3) meshTransforms.meshView);
    vso.viewBitan = mul(bitan, (float3x3) meshTransforms.meshView);
    vso.pos = mul(float4(pos, 1.0f), meshTransforms.meshViewProj);
    vso.texUV = texUV;
    vso.shadowPos = mul(mul(float4(pos, 1.0f), meshTransforms.mesh), shadowTransforms.shadowPos); // Convert Shadow Position to Homogeneous Clip Space.
    return vso;
}