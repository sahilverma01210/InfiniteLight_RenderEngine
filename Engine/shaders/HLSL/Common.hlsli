#include "Packing.hlsli"
#include "Constants.hlsli"

/*
* Convert all XMMATRIX or XMFLOAT4X4 which are Row - major into Column - major matrix which is used by HLSL by default.
* Use transpose() in HLSL or XMMatrixTranspose() in C++ to achieve this.
*/

struct FrameDataHandles
{
    int frameDataIdx;
};

struct CameraConstants
{
    row_major matrix viewMat;
    row_major matrix projectionMat;
    row_major matrix inverseViewMat;
    row_major matrix inverseProjectionMat;
};

ConstantBuffer<FrameDataHandles> frameDataHandles : register(b0);

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
    ConstantBuffer<CameraConstants> cameraConstants = ResourceDescriptorHeap[frameDataHandles.frameDataIdx];
    return cameraConstants.viewMat;
}
matrix GetProjectionMat()
{
    ConstantBuffer<CameraConstants> cameraConstants = ResourceDescriptorHeap[frameDataHandles.frameDataIdx];
    return cameraConstants.projectionMat;
}
matrix GetInverseViewMat()
{
    ConstantBuffer<CameraConstants> cameraConstants = ResourceDescriptorHeap[frameDataHandles.frameDataIdx];
    return cameraConstants.inverseViewMat;
}
matrix GetInverseProjectionMat()
{
    ConstantBuffer<CameraConstants> cameraConstants = ResourceDescriptorHeap[frameDataHandles.frameDataIdx];
    return cameraConstants.inverseProjectionMat;
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

uint GetCubeFaceIndex(float3 v)
{
    float3 vAbs = abs(v);
    uint faceIndex = 0;
    if (vAbs.z >= vAbs.x && vAbs.z >= vAbs.y)
    {
        faceIndex = v.z < 0 ? 5 : 4;
    }
    else if (vAbs.y >= vAbs.x)
    {
        faceIndex = v.y < 0 ? 3 : 2;
    }
    else
    {
        faceIndex = v.x < 0 ? 1 : 0;
    }
    return faceIndex;
}

// Builds a left-handed view matrix (eye looking at target with given up vector).
float4x4 LookAtLH(float3 eye, float3 target, float3 up)
{
    // 1) Compute camera basis vectors
    float3 zaxis = normalize(target - eye); // forward
    float3 xaxis = normalize(cross(up, zaxis)); // right
    float3 yaxis = cross(zaxis, xaxis); // up corrected

    // 2) Fill in view matrix, row by row (HLSL is column-major by default)
    float4 row0 = float4(xaxis.x, yaxis.x, zaxis.x, 0.0f);
    float4 row1 = float4(xaxis.y, yaxis.y, zaxis.y, 0.0f);
    float4 row2 = float4(xaxis.z, yaxis.z, zaxis.z, 0.0f);
    float4 row3 = float4(
        -dot(xaxis, eye),
        -dot(yaxis, eye),
        -dot(zaxis, eye),
         1.0f
    );

    return float4x4(row0, row1, row2, row3);
}

// Constructs a left-handed perspective projection matrix.
float4x4 PerspectiveFovLH(float fovY, float aspect, float znear, float zfar)
{
    float yScale = 1.0f / tan(fovY * 0.5f); // Cotangent of half the vertical FOV
    float xScale = yScale / aspect; // Adjust for aspect ratio
    float zRange = zfar - znear;

    float4x4 result = float4x4(
        xScale, 0.0f, 0.0f, 0.0f,
        0.0f, yScale, 0.0f, 0.0f,
        0.0f, 0.0f, zfar / zRange, 1.0f,
        0.0f, 0.0f, -znear * zfar / zRange, 0.0f
    );

    return result;
}

float4x4 Get360ViewMatrix(uint directionIndex, float3 lightPosition)
{
    switch (directionIndex)
    {
        case 0: // +x			
            return LookAtLH(lightPosition, lightPosition + float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f));
            break;
        case 1: // -x			
            return LookAtLH(lightPosition, lightPosition + float3(-1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f));
            break;
        case 2: // +y			
            return LookAtLH(lightPosition, lightPosition + float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, -1.0f));
            break;
        case 3: // -y			
            return LookAtLH(lightPosition, lightPosition + float3(0.0f, -1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f));
            break;
        case 4: // +z			
            return LookAtLH(lightPosition, lightPosition + float3(0.0f, 0.0f, 1.0f), float3(0.0f, 1.0f, 0.0f));
            break;
        case 5: // -z			
            return LookAtLH(lightPosition, lightPosition + float3(0.0f, 0.0f, -1.0f), float3(0.0f, 1.0f, 0.0f));
            break;
        default:
            return (float4x4) 0;
            break;
    }
}

float4x4 Get360ProjectionMatrix()
{
    return PerspectiveFovLH(M_PI / 2.0f, 1.0f, 0.05f, 400.0f);
}