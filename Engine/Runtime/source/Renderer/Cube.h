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
			XMFLOAT3 normal;
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
				//verts[i].uvCoord = uvs[i];
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

		template<class V>
		static IndexedTriangleList<V> MakeIndependent()
		{
			constexpr float side = 1.0f / 2.0f;

			std::vector<V> vertices(24);
			vertices[0].position = { -side,-side,-side };// 0 near side
			vertices[1].position = { side,-side,-side };// 1
			vertices[2].position = { -side,side,-side };// 2
			vertices[3].position = { side,side,-side };// 3
			vertices[4].position = { -side,-side,side };// 4 far side
			vertices[5].position = { side,-side,side };// 5
			vertices[6].position = { -side,side,side };// 6
			vertices[7].position = { side,side,side };// 7
			vertices[8].position = { -side,-side,-side };// 8 left side
			vertices[9].position = { -side,side,-side };// 9
			vertices[10].position = { -side,-side,side };// 10
			vertices[11].position = { -side,side,side };// 11
			vertices[12].position = { side,-side,-side };// 12 right side
			vertices[13].position = { side,side,-side };// 13
			vertices[14].position = { side,-side,side };// 14
			vertices[15].position = { side,side,side };// 15
			vertices[16].position = { -side,-side,-side };// 16 bottom side
			vertices[17].position = { side,-side,-side };// 17
			vertices[18].position = { -side,-side,side };// 18
			vertices[19].position = { side,-side,side };// 19
			vertices[20].position = { -side,side,-side };// 20 top side
			vertices[21].position = { side,side,-side };// 21
			vertices[22].position = { -side,side,side };// 22
			vertices[23].position = { side,side,side };// 23

			return{
				std::move(vertices),{
					0,2, 1,    2,3,1,
					4,5, 7,    4,7,6,
					8,10, 9,  10,11,9,
					12,13,15, 12,15,14,
					16,17,18, 18,17,19,
					20,23,21, 20,22,23
				}
			};
		}

		template<class V>
		static IndexedTriangleList<V> MakeIndependentTextured()
		{
			auto itl = MakeIndependent<V>();

			itl.vertices[0].uvCoord = { 0.0f,0.0f };
			itl.vertices[1].uvCoord = { 1.0f,0.0f };
			itl.vertices[2].uvCoord = { 0.0f,1.0f };
			itl.vertices[3].uvCoord = { 1.0f,1.0f };
			itl.vertices[4].uvCoord = { 0.0f,0.0f };
			itl.vertices[5].uvCoord = { 1.0f,0.0f };
			itl.vertices[6].uvCoord = { 0.0f,1.0f };
			itl.vertices[7].uvCoord = { 1.0f,1.0f };
			itl.vertices[8].uvCoord = { 0.0f,0.0f };
			itl.vertices[9].uvCoord = { 1.0f,0.0f };
			itl.vertices[10].uvCoord = { 0.0f,1.0f };
			itl.vertices[11].uvCoord = { 1.0f,1.0f };
			itl.vertices[12].uvCoord = { 0.0f,0.0f };
			itl.vertices[13].uvCoord = { 1.0f,0.0f };
			itl.vertices[14].uvCoord = { 0.0f,1.0f };
			itl.vertices[15].uvCoord = { 1.0f,1.0f };
			itl.vertices[16].uvCoord = { 0.0f,0.0f };
			itl.vertices[17].uvCoord = { 1.0f,0.0f };
			itl.vertices[18].uvCoord = { 0.0f,1.0f };
			itl.vertices[19].uvCoord = { 1.0f,1.0f };
			itl.vertices[20].uvCoord = { 0.0f,0.0f };
			itl.vertices[21].uvCoord = { 1.0f,0.0f };
			itl.vertices[22].uvCoord = { 0.0f,1.0f };
			itl.vertices[23].uvCoord = { 1.0f,1.0f };

			return itl;
		}
	};
}