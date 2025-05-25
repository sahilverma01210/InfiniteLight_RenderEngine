#include "Common.hlsli"
#include "Scene.hlsli"

struct CubeFace
{
    uint index;
};

struct Light
{
    float3 position;
};

ConstantBuffer<CubeFace> cubeFace : register(b3);
ConstantBuffer<Light> light : register(b4);

float4 main(float3 pos : Position) : SV_Position
{
    //float4x4 lightViewProjection = transpose(mul(Get360ViewMatrix(cubeFace.index, light.position), Get360ProjectionMatrix()));
    //float4 posWS = mul(float4(pos, 1.0f), GetMeshMat());
    //float4 posLS = mul(posWS, lightViewProjection);    
    //return posLS;
    
    float4x4 meshViewProj = mul(mul(GetMeshMat(), Get360ViewMatrix(cubeFace.index, light.position)), Get360ProjectionMatrix());
    return mul(float4(pos, 1.0f), meshViewProj);
}