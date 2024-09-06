#pragma once
#include <vector>
#include <DirectXMath.h>

template<class T>
class IndexedTriangleList
{
public:
	IndexedTriangleList() = default;

	IndexedTriangleList(std::vector<T> verts_in, std::vector<USHORT> indices_in)
		:
		vertices(std::move(verts_in)),
		indices(std::move(indices_in))
	{
		assert(vertices.size() > 2);
		assert(indices.size() % 3 == 0);
	}

	void Transform(DirectX::FXMMATRIX matrix)
	{
		for (auto& v : vertices)
		{
			const DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&v.position);
			DirectX::XMStoreFloat3(
				&v.position,
				DirectX::XMVector3Transform(pos, matrix)
			);
		}
	}

	// asserts face-independent vertices w/ normals cleared to zero
	void SetNormalsIndependentFlat() noexcept
	{
		using namespace DirectX;
		assert(indices.size() % 3 == 0 && indices.size() > 0);
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			auto& v0 = vertices[indices[i]];
			auto& v1 = vertices[indices[i + 1]];
			auto& v2 = vertices[indices[i + 2]];
			const auto p0 = XMLoadFloat3(&v0.position);
			const auto p1 = XMLoadFloat3(&v1.position);
			const auto p2 = XMLoadFloat3(&v2.position);

			const auto normal = XMVector3Normalize(XMVector3Cross((p1 - p0), (p2 - p0)));

			XMStoreFloat3(&v0.normal, normal);
			XMStoreFloat3(&v1.normal, normal);
			XMStoreFloat3(&v2.normal, normal);
		}
	}

public:
	std::vector<T> vertices;
	std::vector<USHORT> indices;
};