#pragma once
#include "../_External/dx12/directX12.h"
#include "../_External/common.h"
#include "../_External/assimp/assimp.h"

namespace Renderer
{
	struct VertexStruct
	{
		Vector3 position;
		Vector3 normal;
		Vector2 texCoord;
		Vector3 tangent;
		Vector3 bitangent;
		VertexStruct() = default;
		VertexStruct(const Vector3& pos, const Vector3& norm, const Vector2& tex, const Vector3& tan, const Vector3& bitan)
			: position(pos), normal(norm), texCoord(tex), tangent(tan), bitangent(bitan) {
		}
	};

	class IndexedTriangleList
	{
	public:
		IndexedTriangleList() = default;
		IndexedTriangleList(std::vector<VertexStruct> verts_in, std::vector<USHORT> indices_in)
			:
			vertices(std::move(verts_in)),
			indices(std::move(indices_in))
		{
			assert(vertices.size() > 2);
			assert(indices.size() % 3 == 0);
		}
		void Transform(FXMMATRIX matrix)
		{
			for (int i = 0; i < vertices.size(); i++)
			{
				auto& pos = vertices[i].position;
				XMStoreFloat3(
					&pos,
					XMVector3Transform(XMLoadFloat3(&pos), matrix)
				);
			}
		}
		// asserts face-independent vertices w/ normals cleared to zero
		void SetNormalsIndependentFlat() noexcept(!IS_DEBUG)
		{
			using namespace DirectX;
			for (size_t i = 0; i < indices.size(); i += 3)
			{
				auto v0 = vertices[indices[i]];
				auto v1 = vertices[indices[i + 1]];
				auto v2 = vertices[indices[i + 2]];
				const auto p0 = XMLoadFloat3(&v0.position);
				const auto p1 = XMLoadFloat3(&v1.position);
				const auto p2 = XMLoadFloat3(&v2.position);

				const auto n = XMVector3Normalize(XMVector3Cross((p1 - p0), (p2 - p0)));

				XMStoreFloat3(&v0.normal, n);
				XMStoreFloat3(&v1.normal, n);
				XMStoreFloat3(&v2.normal, n);
			}
		}

	public:
		std::vector<VertexStruct> vertices;
		std::vector<USHORT> indices;
	};
}