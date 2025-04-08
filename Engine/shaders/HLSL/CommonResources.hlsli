struct MeshTransforms
{
    matrix mesh;
    matrix meshView;
    matrix meshViewProj;
};

struct MeshConstants
{
    int materialTypeId;
    int materialIdx;
};

ConstantBuffer<MeshTransforms> meshTransforms : register(b0, space1);
ConstantBuffer<MeshConstants> meshConstants : register(b1, space1);