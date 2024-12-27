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

	void Mesh::Submit(size_t channel, FXMMATRIX accumulatedTranform) const noexcept(!IS_DEBUG)
	{
		XMStoreFloat4x4(&m_transform, accumulatedTranform);
		ILMesh::Submit(channel);
	}

	void Mesh::SetTransform(D3D12RHI& gfx, std::string targetPass) const noexcept(!IS_DEBUG)
	{
		const auto model = XMLoadFloat4x4(&m_transform);
		const auto modelView = model * m_cameraMatrix;

		/*
		* Convert all XMMATRIX or XMFLOAT4X4 which are Row - major into Column - major matrix which is used by HLSL by default.
		* Use XMMatrixTranspose() to achieve this.
		*/
		m_transforms = {
			XMMatrixTranspose(model),
			XMMatrixTranspose(modelView),
			XMMatrixTranspose(
				modelView *
				m_projectionMatrix
			)
		};

		gfx.Set32BitRootConstants(0, sizeof(m_transforms) / 4, &m_transforms);
	}
}