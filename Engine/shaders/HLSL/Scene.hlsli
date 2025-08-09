#include "Common.hlsli"

struct Instance
{
    row_major matrix worldMatrix;
    row_major matrix inverseWorldMatrix;
    uint instanceId;
    uint meshIndex;
    uint materialIndex;
};

struct Mesh
{
    uint bufferIdx;
    uint positionsOffset;
    uint uvsOffset;
    uint normalsOffset;
    uint tangentsOffset;
    uint indicesOffset;
    uint indicesCount;
    uint verticesCount;
};

struct Material
{
    float3 pbrBaseColorFactor;
    float pbrMetallicFactor;
    float pbrRoughnessFactor;
    float gltfAlphaCutoff;
    int gltfAlphaMode; // 0: Opaque, 1: Blend, 2: Mask
    int diffuseIdx;
    int normalIdx;
    int roughnessMetallicIdx;
};

Instance GetInstanceData(uint instanceId)
{
    StructuredBuffer<Instance> instances = ResourceDescriptorHeap[frameData.instancesIdx];
    return instances[instanceId];
}

Mesh GetMeshData(uint meshIdx)
{
    StructuredBuffer<Mesh> meshes = ResourceDescriptorHeap[frameData.meshesIdx];
    return meshes[meshIdx];
}

Material GetMaterialData(uint materialIdx)
{
    StructuredBuffer<Material> materials = ResourceDescriptorHeap[frameData.materialsIdx];
    return materials[materialIdx];
}

template<typename T>
T LoadMeshBuffer(uint bufferIdx, uint bufferOffset, uint vertexId)
{
    ByteAddressBuffer meshBuffer = ResourceDescriptorHeap[NonUniformResourceIndex(bufferIdx)];
    return meshBuffer.Load < T > (bufferOffset + sizeof(T) * vertexId);
}

struct VertexData
{
    float3 pos;
    float2 uv;
    float3 nor;
};

float3 Interpolate(in float3 x0, in float3 x1, in float3 x2, float2 bary)
{
    return x0 * (1.0f - bary.x - bary.y) + bary.x * x1 + bary.y * x2;
}
float2 Interpolate(in float2 x0, in float2 x1, in float2 x2, float2 bary)
{
    return x0 * (1.0f - bary.x - bary.y) + bary.x * x1 + bary.y * x2;
}

VertexData LoadVertexData(Mesh meshData, uint triangleIndex, float2 barycentrics)
{
    uint i0 = LoadMeshBuffer<uint>(meshData.bufferIdx, meshData.indicesOffset, 3 * triangleIndex + 0);
    uint i1 = LoadMeshBuffer<uint>(meshData.bufferIdx, meshData.indicesOffset, 3 * triangleIndex + 1);
    uint i2 = LoadMeshBuffer<uint>(meshData.bufferIdx, meshData.indicesOffset, 3 * triangleIndex + 2);

    float3 pos0 = LoadMeshBuffer<float3>(meshData.bufferIdx, meshData.positionsOffset, i0);
    float3 pos1 = LoadMeshBuffer<float3>(meshData.bufferIdx, meshData.positionsOffset, i1);
    float3 pos2 = LoadMeshBuffer<float3>(meshData.bufferIdx, meshData.positionsOffset, i2);
    float3 pos = Interpolate(pos0, pos1, pos2, barycentrics);

    float2 uv0 = LoadMeshBuffer<float2>(meshData.bufferIdx, meshData.uvsOffset, i0);
    float2 uv1 = LoadMeshBuffer<float2>(meshData.bufferIdx, meshData.uvsOffset, i1);
    float2 uv2 = LoadMeshBuffer<float2>(meshData.bufferIdx, meshData.uvsOffset, i2);
    float2 uv = Interpolate(uv0, uv1, uv2, barycentrics);

    float3 nor0 = LoadMeshBuffer<float3>(meshData.bufferIdx, meshData.normalsOffset, i0);
    float3 nor1 = LoadMeshBuffer<float3>(meshData.bufferIdx, meshData.normalsOffset, i1);
    float3 nor2 = LoadMeshBuffer<float3>(meshData.bufferIdx, meshData.normalsOffset, i2);
    float3 nor = normalize(Interpolate(nor0, nor1, nor2, barycentrics));

    VertexData vertex = (VertexData) 0;
    vertex.pos = pos;
    vertex.uv = uv;
    vertex.nor = nor;
    return vertex;
}