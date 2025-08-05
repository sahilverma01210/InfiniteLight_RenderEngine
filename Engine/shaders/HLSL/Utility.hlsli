#ifndef _UTILITY_
#define _UTILITY_

#include "Constants.hlsli"

float4 GetRotationToZAxis(float3 input)
{
    if (input.z < -0.99999f)
    {
        return float4(1.0f, 0.0f, 0.0f, 0.0f);
    }
    return normalize(float4(input.y, -input.x, 0.0f, 1.0f + input.z));
}
float4 InvertRotation(float4 q)
{
    return float4(-q.x, -q.y, -q.z, q.w);
}
float3 RotatePoint(float4 q, float3 v)
{
    float3 qAxis = float3(q.x, q.y, q.z);
    return 2.0f * dot(qAxis, v) * qAxis + (q.w * q.w - dot(qAxis, qAxis)) * v + 2.0f * q.w * cross(qAxis, v);
}
float3x3 AngleAxis3x3(float angle, float3 axis)
{
	// Rotation with angle (in radians) and axis
    float c, s;
    sincos(angle, s, c);

    float t = 1 - c;
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;

    return float3x3(
		t * x * x + c, t * x * y - s * z, t * x * z + s * y,
		t * x * y + s * z, t * y * y + c, t * y * z - s * x,
		t * x * z - s * y, t * y * z + s * x, t * z * z + c
		);
}

float3 GetPerpendicularVector(float3 u)
{
	// Utility function to get a vector perpendicular to an input vector 
	// (from "Efficient Construction of Perpendicular Vectors Without Branching")
    float3 a = abs(u);
    uint xm = ((a.x - a.y) < 0 && (a.x - a.z) < 0) ? 1 : 0;
    uint ym = (a.y - a.z) < 0 ? (1 ^ xm) : 0;
    uint zm = 1 ^ (xm | ym);
    return cross(u, float3(xm, ym, zm));
}
float3x3 GetTangentMatrix(float3 N)
{
    const float3 B = GetPerpendicularVector(N);
    const float3 T = cross(B, N);
    return float3x3(T, B, N);
}
float3 TangentToWorld(float3 v, float3 N)
{
    float3x3 tangentBasis = GetTangentMatrix(N);
    return mul(v, tangentBasis);
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

bool IsSaturated(float value)
{
    return value == saturate(value);
}
bool IsSaturated(float2 value)
{
    return IsSaturated(value.x) && IsSaturated(value.y);
}
bool IsSaturated(float3 value)
{
    return IsSaturated(value.x) && IsSaturated(value.y) && IsSaturated(value.z);
}
bool IsSaturated(float4 value)
{
    return IsSaturated(value.x) && IsSaturated(value.y) && IsSaturated(value.z) && IsSaturated(value.w);
}

float Max(float2 v)
{
    return max(v.x, v.y);
}
float Max(float3 v)
{
    return max(Max(v.xy), v.z);
}
float Max(float4 v)
{
    return max(Max(v.xyz), v.w);
}
float Min(float2 v)
{
    return min(v.x, v.y);
}
float Min(float3 v)
{
    return min(Min(v.xy), v.z);
}
float Min(float4 v)
{
    return min(Min(v.xyz), v.w);
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

#endif

