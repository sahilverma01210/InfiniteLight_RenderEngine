#include "Packing.hlsli"
#include "Utility.hlsli"

/*
* Convert all XMMATRIX or XMFLOAT4X4 which are Row - major into Column - major matrix which is used by HLSL by default.
* Use transpose() in HLSL or XMMatrixTranspose() in C++ to achieve this.
*/

#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

struct FrameData
{
    float2 resolution;
    int lightDataIdx;
    int cameraDataIdx;
    int envMapIdx;
    int accelStructIdx;
    int instancesIdx;
    int meshesIdx;
    int materialsIdx;
    uint frameCount;
    uint lightCount;    
};

struct Light
{
    float3 position;
    float3 direction;
    float3 viewPos;
    float3 ambient;
    float3 diffuseColor;
    float diffuseIntensity;
    float range;
    int shadowDepthIdx;
    int type;
};

struct Camera
{
    row_major matrix viewMat;
    row_major matrix projectionMat;
    row_major matrix inverseViewMat;
    row_major matrix inverseProjectionMat;
    row_major matrix inverseViewProjectionMat;
    float3 position;
};

ConstantBuffer<FrameData> frameData : register(b0);

SamplerState LinearWrapSampler : register(s0);
SamplerState LinearClampSampler : register(s1);
SamplerState LinearBorderSampler : register(s2);

SamplerState PointWrapSampler : register(s3);
SamplerState PointClampSampler : register(s4);
SamplerState PointBorderSampler : register(s5);

SamplerComparisonState ShadowClampSampler : register(s6);
SamplerComparisonState ShadowWrapSampler : register(s7);

SamplerState LinearMirrorSampler : register(s8);
SamplerState PointMirrorSampler : register(s9);

matrix GetViewMat()
{
    StructuredBuffer<Camera> cameras = ResourceDescriptorHeap[frameData.cameraDataIdx];
    return cameras[0].viewMat;
}
matrix GetProjectionMat()
{
    StructuredBuffer<Camera> cameras = ResourceDescriptorHeap[frameData.cameraDataIdx];
    return cameras[0].projectionMat;
}
matrix GetInverseViewMat()
{
    StructuredBuffer<Camera> cameras = ResourceDescriptorHeap[frameData.cameraDataIdx];
    return cameras[0].inverseViewMat;
}
matrix GetInverseProjectionMat()
{
    StructuredBuffer<Camera> cameras = ResourceDescriptorHeap[frameData.cameraDataIdx];
    return cameras[0].inverseProjectionMat;
}

matrix GetInverseViewProjectionMat()
{
    StructuredBuffer<Camera> cameras = ResourceDescriptorHeap[frameData.cameraDataIdx];
    return cameras[0].inverseViewProjectionMat;
}

float3 GetCameraPosition()
{
    StructuredBuffer<Camera> cameras = ResourceDescriptorHeap[frameData.cameraDataIdx];
    return cameras[0].position;
}

static float3 GetViewPosition(float2 texcoord, float depth)
{
    float4 clipSpaceLocation;
    clipSpaceLocation.xy = texcoord * 2.0f - 1.0f;
    clipSpaceLocation.y *= -1;
    clipSpaceLocation.z = depth;
    clipSpaceLocation.w = 1.0f;
    float4 homogenousLocation = mul(clipSpaceLocation, GetInverseProjectionMat());
    return homogenousLocation.xyz / homogenousLocation.w;
}

Light GetLightData(uint index)
{
    StructuredBuffer<Light> lightBuffer = ResourceDescriptorHeap[frameData.lightDataIdx];
    return lightBuffer[index];
}
