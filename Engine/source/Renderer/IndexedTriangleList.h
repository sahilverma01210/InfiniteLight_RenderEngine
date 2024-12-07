#pragma once
#include "../_External/common.h"

#include "Vertex.h"

namespace Renderer
{
	class IndexedTriangleList
	{
	public:
		IndexedTriangleList() = default;
		IndexedTriangleList(VertexRawBuffer verts_in, std::vector<USHORT> indices_in)
			:
			vertices(std::move(verts_in)),
			indices(std::move(indices_in))
		{
			assert(vertices.Size() > 2);
			assert(indices.size() % 3 == 0);
		}
		void Transform(FXMMATRIX matrix)
		{
			using Elements = VertexLayout::ElementType;
			for (int i = 0; i < vertices.Size(); i++)
			{
				auto& pos = vertices[i].Attr<Elements::Position3D>();
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
			using Type = VertexLayout::ElementType;
			for (size_t i = 0; i < indices.size(); i += 3)
			{
				auto v0 = vertices[indices[i]];
				auto v1 = vertices[indices[i + 1]];
				auto v2 = vertices[indices[i + 2]];
				const auto p0 = XMLoadFloat3(&v0.Attr<Type::Position3D>());
				const auto p1 = XMLoadFloat3(&v1.Attr<Type::Position3D>());
				const auto p2 = XMLoadFloat3(&v2.Attr<Type::Position3D>());

				const auto n = XMVector3Normalize(XMVector3Cross((p1 - p0), (p2 - p0)));

				XMStoreFloat3(&v0.Attr<Type::Normal>(), n);
				XMStoreFloat3(&v1.Attr<Type::Normal>(), n);
				XMStoreFloat3(&v2.Attr<Type::Normal>(), n);
			}
		}

	public:
		VertexRawBuffer vertices;
		std::vector<USHORT> indices;
	};
}