struct Transforms
{
    row_major matrix meshMat;
    row_major matrix meshInvMat;
};

struct MeshConstants
{
    int materialIdx;
};

ConstantBuffer<Transforms> transforms : register(b1);
ConstantBuffer<MeshConstants> meshConstants : register(b2);

matrix GetMeshMat()
{
    return transforms.meshMat;
}

matrix GetMeshInvMat()
{
    return transforms.meshInvMat;
}