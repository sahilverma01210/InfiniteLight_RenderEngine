#pragma once
#include "IndexedTriangleList.h"
#include <DirectXMath.h>

namespace Renderer
{
	class Cube
	{
	public:

		struct Vertex
		{
			XMFLOAT3 position;
			XMFLOAT2 uvCoord;
		};

		template<class V>
		static IndexedTriangleList<V> Make()
		{
			namespace dx = DirectX;

			std::vector<dx::XMFLOAT3> vertices;
			vertices.emplace_back(-1.0f, -1.0f, -1.0f); // 0
			vertices.emplace_back(-1.0f, 1.0f, -1.0f); // 1
			vertices.emplace_back(1.0f, 1.0f, -1.0f); // 2
			vertices.emplace_back(1.0f, -1.0f, -1.0f); // 3
			vertices.emplace_back(-1.0f, -1.0f, 1.0f); // 4
			vertices.emplace_back(-1.0f, 1.0f, 1.0f); // 5
			vertices.emplace_back(1.0f, 1.0f, 1.0f); // 6
			vertices.emplace_back(1.0f, -1.0f, 1.0f); // 7

			std::vector<dx::XMFLOAT2> uvs;
			uvs.emplace_back(0.f, 0.f); // 0
			uvs.emplace_back(0.f, 1.f); // 1
			uvs.emplace_back(0.f, 1.f); // 2
			uvs.emplace_back(1.f, 0.f); // 3
			uvs.emplace_back(0.f, 1.f); // 4
			uvs.emplace_back(0.f, 0.f); // 5
			uvs.emplace_back(1.f, 0.f); // 6
			uvs.emplace_back(1.f, 1.f); // 7

			std::vector<V> verts(vertices.size());
			for (size_t i = 0; i < vertices.size(); i++)
			{
				verts[i].position = vertices[i];
				verts[i].uvCoord = uvs[i];
			}
			return{
				std::move(verts),{
					0, 1, 2, 0, 2, 3,
					4, 6, 5, 4, 7, 6,
					4, 5, 1, 4, 1, 0,
					3, 2, 6, 3, 6, 7,
					1, 5, 6, 1, 6, 2,
					4, 0, 3, 4, 3, 7
				}
			};
		}
	};
}