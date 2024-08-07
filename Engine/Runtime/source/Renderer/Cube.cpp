#include "Cube.h"

namespace Renderer
{
    // Define the geometry for a triangle.
    Vertex cubeVertices[] =
    {
        { {-1.0f, -1.0f, -1.0f}, { 0.f, 0.f } }, // 0 
        { {-1.0f,  1.0f, -1.0f}, { 0.f, 1.f } }, // 1 
        { {1.0f,  1.0f, -1.0f}, { 1.f, 1.f } }, // 2 
        { {1.0f, -1.0f, -1.0f}, { 1.f, 0.f } }, // 3 
        { {-1.0f, -1.0f,  1.0f}, { 0.f, 1.f } }, // 4 
        { {-1.0f,  1.0f,  1.0f}, { 0.f, 0.f } }, // 5 
        { {1.0f,  1.0f,  1.0f}, { 1.f, 0.f } }, // 6 
        { {1.0f, -1.0f,  1.0f}, { 1.f, 1.f } }  // 7 
    };

    // Cube indices (Cube Vertex Order to form Triangles)
    USHORT cubeIndices[] =
    {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7
    };

	Cube::Cube() : Mesh(sizeof(cubeVertices), sizeof(cubeIndices), cubeVertices, cubeIndices)
	{

	}
}