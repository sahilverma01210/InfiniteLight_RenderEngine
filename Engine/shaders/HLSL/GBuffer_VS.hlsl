#include "CommonResources.hlsli"

struct VSOut
{
    float2 texUV : Texcoord;
    float4 posWS : PositionWorld;
    float3 posVS : PositionView;
    float4 posCS : SV_Position;
    float3 normalVS : Normal;
    float3 tanVS : Tangent;
    float3 bitanVS : Bitangent;
};

VSOut main(float3 pos : Position, float3 norm : Normal, float2 texUV : Texcoord, float3 tan : Tangent, float3 bitan : Bitangent)
{
    float4x4 mesh = GetMeshMat();
    float4x4 meshView = mul(mesh, GetCameraMat());
    float4x4 meshViewProj = mul(meshView, GetProjectionMat());
    
    VSOut vso;
    vso.texUV = texUV;
    vso.posWS = mul(float4(pos, 1.0f), mesh); // World space position
    vso.posVS = (float3) mul(float4(pos, 1.0f), meshView); // View space position
    vso.posCS = mul(float4(pos, 1.0f), meshViewProj); // Clip space position
    vso.normalVS = mul(norm, (float3x3) meshView); // View space normal
    vso.tanVS = mul(tan, (float3x3) meshView); // View space tangent
    vso.bitanVS = mul(bitan, (float3x3) meshView); // View space bitangent
    return vso;
}