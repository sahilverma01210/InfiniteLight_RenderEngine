#include "Mesh.h"

namespace Renderer
{
	Mesh::Mesh(D3D12RHI& gfx, std::shared_ptr<Material> material, const aiMesh& mesh, float scale) noexcept(!IS_DEBUG)
		: m_material(material)
	{
		m_vtxLayout.Append(VertexLayout::Position3D);
		m_vtxLayout.Append(VertexLayout::Normal);
		m_vtxLayout.Append(VertexLayout::Texture2D);
		m_vtxLayout.Append(VertexLayout::Tangent);
		m_vtxLayout.Append(VertexLayout::Bitangent);

		m_renderEffects["shadow_map"] = true;
		m_renderEffects["g_buffer"] = true;
		m_renderEffects["object_flat"] = false;

		m_materialIdx = material->GetMaterialHandle();

		ApplyMesh(gfx, MakeVertices(gfx, mesh, scale), MakeIndices(gfx, mesh));
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

	void Mesh::Update(FXMMATRIX accumulatedTranform) const noexcept(!IS_DEBUG)
	{
		XMStoreFloat4x4(&m_transform, accumulatedTranform);
		m_transforms.meshMat = XMLoadFloat4x4(&m_transform);
		m_transforms.meshInvMat = XMMatrixInverse(nullptr, m_transforms.meshMat);
	}
}