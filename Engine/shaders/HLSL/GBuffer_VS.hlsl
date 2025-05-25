#include "Common.hlsli"
#include "Scene.hlsli"

struct VSOut
{
    float2 texUV : Texcoord;
    float3 normalWS : Normal;
    float3 tanWS : Tangent;
    float3 bitanWS : Bitangent;
    float4 posCS : SV_Position;
};

VSOut main(float3 pos : Position, float3 norm : Normal, float2 texUV : Texcoord, float3 tan : Tangent, float3 bitan : Bitangent)
{
    float4x4 mesh = GetMeshMat();
    float4x4 meshInv = GetMeshInvMat();
    
    float4x4 meshViewProj = mul(mul(GetMeshMat(), GetViewMat()), GetProjectionMat());
    
    VSOut vso;
    vso.texUV = texUV;
    vso.normalWS = mul(norm, (float3x3) transpose(meshInv)); // View space normal
    vso.tanWS = mul(tan, (float3x3) mesh); // View space tangent
    vso.bitanWS = mul(bitan, (float3x3) mesh); // View space bitangent
    vso.posCS = mul(float4(pos, 1.0f), meshViewProj); // Clip space position
    return vso;
}