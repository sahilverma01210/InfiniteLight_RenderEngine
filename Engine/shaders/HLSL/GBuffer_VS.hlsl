#include "Scene.hlsli"

struct VSOut
{
    float2 texUV : Texcoord;
    float3 normalWS : Normal;
    float3 tanWS : Tangent;
    float3 bitanWS : Bitangent;
    float4 posCS : SV_Position;
};

struct CurrentInstance
{
    uint index;
};

ConstantBuffer<CurrentInstance> currentInstance : register(b1);

VSOut main(float3 pos : Position, float3 norm : Normal, float2 texUV : Texcoord, float3 tan : Tangent, float3 bitan : Bitangent)
{
    Instance instance = GetInstanceData(currentInstance.index);
    
    float4x4 meshViewProj = mul(mul(instance.worldMatrix, GetViewMat()), GetProjectionMat());
    
    VSOut vso;
    vso.texUV = texUV;
    vso.normalWS = mul(norm, (float3x3) transpose(instance.inverseWorldMatrix)); // View space normal
    vso.tanWS = mul(tan, (float3x3) instance.worldMatrix); // View space tangent
    vso.bitanWS = mul(bitan, (float3x3) instance.worldMatrix); // View space bitangent
    vso.posCS = mul(float4(pos, 1.0f), meshViewProj); // Clip space position
    return vso;
}