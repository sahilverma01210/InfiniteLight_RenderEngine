#include "CommonResources.hlsl"
#include "LightShadowUtils.hlsl"

struct PhongCB
{
    int lightConstIdx;
    int texConstIdx;
    int shadowTexIdx;
    int diffTexIdx;
    int normTexIdx;
    int specTexIdx;
    int solidConstIdx;
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

VSOut main(float3 pos : Position, float3 norm : Normal, float2 texUV : Texcoord, float3 tan : Tangent, float3 bitan : Bitangent)
{
    ConstantBuffer<PhongCB> phongCB = ResourceDescriptorHeap[meshConstants.materialIdx];
    ConstantBuffer<PointLightProps> pointLightCB = ResourceDescriptorHeap[phongCB.lightConstIdx];
    
    matrix shadowViewProj =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        -pointLightCB.pos.x, -pointLightCB.pos.y, -pointLightCB.pos.z, 1,
    };
        
    VSOut vso;
    vso.viewPos = (float3) mul(float4(pos, 1.0f), meshTransforms.meshView);
    vso.viewNormal = mul(norm, (float3x3) meshTransforms.meshView);
    vso.viewTan = mul(tan, (float3x3) meshTransforms.meshView);
    vso.viewBitan = mul(bitan, (float3x3) meshTransforms.meshView);
    vso.pos = mul(float4(pos, 1.0f), meshTransforms.meshViewProj);
    vso.texUV = texUV;
    vso.shadowPos = mul(mul(float4(pos, 1.0f), meshTransforms.mesh), shadowViewProj); // Convert Shadow Position to Homogeneous Clip Space.
    return vso;
}