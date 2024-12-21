#include "Mesh.h"

namespace Renderer
{
	Mesh::Mesh(D3D12RHI& gfx, ImportMaterial& material, const aiMesh& mesh, float scale) noexcept(!IS_DEBUG)
	{
		m_vtxLayout = material.GetVertexLayout();

		ApplyMesh(gfx, MakeVertices(gfx, mesh, scale), MakeIndices(gfx, mesh));

		ApplyMaterial(gfx, material, true);
	}

	VertexRawBuffer Mesh::MakeVertices(D3D12RHI& gfx, const aiMesh& mesh, float scale) const noexcept(!IS_DEBUG)
	{
		VertexRawBuffer vertex{ m_vtxLayout,mesh };
		if (scale != 1.0f)
		{
			for (auto i = 0u; i < vertex.Size(); i++)
			{
				XMFLOAT3& pos = vertex[i].Attr<VertexLayout::ElementType::Position3D>();
				pos.x *= scale;
				pos.y *= scale;
				pos.z *= scale;
			}
		}

		return vertex;
	}

	std::vector<USHORT> Mesh::MakeIndices(D3D12RHI& gfx, const aiMesh& mesh) const noexcept(!IS_DEBUG)
	{
		std::vector<USHORT> indices;
		indices.reserve(mesh.mNumFaces * 3);
		for (unsigned int i = 0; i < mesh.mNumFaces; i++)
		{
			const auto& face = mesh.mFaces[i];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}

		return indices;
	}

	void Mesh::Submit(size_t channels, FXMMATRIX accumulatedTranform) const noexcept(!IS_DEBUG)
	{
		XMStoreFloat4x4(&m_transform, accumulatedTranform);
		ILMesh::Submit(channels);
	}

	XMMATRIX Mesh::GetTransformXM() const noexcept(!IS_DEBUG)
	{
		return XMLoadFloat4x4(&m_transform);
	}
}