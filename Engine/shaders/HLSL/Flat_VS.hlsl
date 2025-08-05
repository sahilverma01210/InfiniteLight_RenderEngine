#include "Common.hlsli"

struct Transforms
{
    row_major matrix meshMat;
};

ConstantBuffer<Transforms> transforms : register(b1);

float4 main(float3 pos : Position, float3 norm : Normal, float2 texUV : Texcoord, float3 tan : Tangent, float3 bitan : Bitangent) : SV_Position
{
    float4x4 meshViewProj = mul(mul(transforms.meshMat, GetViewMat()), GetProjectionMat());
    
    return mul(float4(pos, 1.0f), meshViewProj);
}