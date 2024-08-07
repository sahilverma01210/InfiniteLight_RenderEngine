#include "Mesh.h"

namespace Renderer
{
    Mesh::Mesh(UINT nVertices, UINT nIndices, Vertex* pVertices, USHORT* pIndices)
    {
        numVertices = nVertices;
        numIndices = nIndices;
        vertices = pVertices;
        indices = pIndices;
    }

    UINT Mesh::GetNumVertices()
    {
        return numVertices;
    }
    UINT Mesh::GetNumIndices()
    {
        return numIndices;
    }
    Vertex* Mesh::GetVertices()
    {
        return vertices;
    }
    USHORT* Mesh::GetIndices()
    {
        return indices;
    }
}