#pragma once

#include "../Common/framework.h"
#include "d3dx12.h"

using namespace DirectX;

namespace Renderer
{
    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT2 uvCoord;
    };

    class Mesh
    {
    public:
        Mesh(UINT nVertices, UINT nIndices, Vertex* pVertices, USHORT* pIndices);
        UINT GetNumVertices();
        UINT GetNumIndices();
        Vertex* GetVertices();
        USHORT* GetIndices();
    private:
        UINT numVertices;
        UINT numIndices;
        Vertex* vertices;
        USHORT* indices;
    };
}
