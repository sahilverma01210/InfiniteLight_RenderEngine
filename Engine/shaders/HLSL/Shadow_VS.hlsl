#include "Lighting.hlsli"

struct Transforms
{
    row_major matrix meshMat;
};

struct ShadowIndices
{
    uint currentLightIndex;
    uint currentfaceIndex;
};

ConstantBuffer<Transforms> transforms : register(b1);
ConstantBuffer<ShadowIndices> shadowIndices : register(b2);

float4 main(float3 pos : Position, float3 norm : Normal, float2 texUV : Texcoord, float3 tan : Tangent, float3 bitan : Bitangent) : SV_Position
{
    float3 lightPos = GetLightData(shadowIndices.currentLightIndex).position;
    
    float4x4 meshViewProj = mul(mul(transforms.meshMat, Get360ViewMatrix(shadowIndices.currentfaceIndex, lightPos)), Get360ProjectionMatrix());
    return mul(float4(pos, 1.0f), meshViewProj);
}