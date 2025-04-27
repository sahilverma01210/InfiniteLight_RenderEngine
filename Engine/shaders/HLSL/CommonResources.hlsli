
/*
* Convert all XMMATRIX or XMFLOAT4X4 which are Row - major into Column - major matrix which is used by HLSL by default.
* Use transpose() in HLSL or XMMatrixTranspose() in C++ to achieve this.
*/

struct Transforms
{
    float4x4 meshMat;
    float4x4 cameraMat;
    float4x4 projectionMat;
};

struct MeshConstants
{
    int materialTypeId;
    int materialIdx;
};

ConstantBuffer<Transforms> transforms : register(b0);
ConstantBuffer<MeshConstants> meshConstants : register(b1);

float4x4 GetMeshMat()
{
    return transpose(transforms.meshMat);
}
float4x4 GetCameraMat()
{
    return transpose(transforms.cameraMat);
}
float4x4 GetProjectionMat()
{
    return transpose(transforms.projectionMat);
}