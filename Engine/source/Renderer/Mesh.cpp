#include "Mesh.h"

namespace Renderer
{
	Mesh::Mesh(D3D12RHI& gfx, const aiMesh& mesh, float scale) noexcept(!IS_DEBUG)
	{
		m_renderEffects["shadow_map"] = true;
		m_renderEffects["g_buffer"] = true;
		m_renderEffects["object_flat"] = false;

		m_drawData.vertices = MakeVertices(mesh, scale);
		m_drawData.indices = MakeIndices(mesh);
		m_drawData.vertexSizeInBytes = m_drawData.vertices.size() * sizeof(m_drawData.vertices[0]);
		m_drawData.indexSizeInBytes = m_drawData.indices.size() * sizeof(m_drawData.indices[0]);
		m_drawData.vertexStrideInBytes = sizeof(VertexStruct);

		if (!gfx.IsRayTracingEnabled())
		{
			m_drawData.vertexBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, m_drawData.vertices.data(), m_drawData.vertexSizeInBytes));
			m_drawData.indexBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, m_drawData.indices.data(), m_drawData.indexSizeInBytes));
		}
	}

	std::vector<VertexStruct> Mesh::MakeVertices(const aiMesh& mesh, float scale) const noexcept(!IS_DEBUG)
	{
		std::vector<VertexStruct> vertices;
		vertices.reserve(mesh.mNumVertices);
		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			const auto& vertex = mesh.mVertices[i];
			const auto& normal = mesh.mNormals[i];
			const auto& tangent = mesh.mTangents[i];
			const auto& bitangent = mesh.mBitangents[i];
			const auto& uv = mesh.mTextureCoords[0][i];

			vertices.emplace_back(
				Vector3(vertex.x * scale, vertex.y * scale, vertex.z * scale),
				Vector3(normal.x, normal.y, normal.z),
				Vector2(uv.x, uv.y),
				Vector3(tangent.x, tangent.y, tangent.z),
				Vector3(bitangent.x, bitangent.y, bitangent.z)
			);
		}

		return vertices;
	}

	std::vector<USHORT> Mesh::MakeIndices(const aiMesh& mesh) const noexcept(!IS_DEBUG)
	{
		std::vector<USHORT> indices;
		indices.reserve(mesh.mNumFaces * 3);
		for (unsigned int i = 0; i < mesh.mNumFaces; i++)
		{
			const auto& face = mesh.mFaces[i];
			assert(face.mNumIndices == 3);
			indices.emplace_back(face.mIndices[0]);
			indices.emplace_back(face.mIndices[1]);
			indices.emplace_back(face.mIndices[2]);
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