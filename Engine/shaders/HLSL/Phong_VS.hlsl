#include "CommonResources.hlsl"

struct PhongCB
{
    int shadowConstIdx;
    int lightConstIdx;
    int texConstIdx;
    int shadowTexIdx;
    int diffTexIdx;
    int normTexIdx;
    int specTexIdx;
    int solidConstIdx;
};

struct ShadowTransforms
{
    matrix shadowPos;
};

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
    ConstantBuffer<PhongCB> phongCB = ResourceDescriptorHeap[meshConstants.materialIdx];
    ConstantBuffer<ShadowTransforms> shadowTransforms = ResourceDescriptorHeap[phongCB.shadowConstIdx];
    
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