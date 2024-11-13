#include "Mesh.h"

namespace Renderer
{
	// Mesh Definitions.

	//Mesh::Mesh(D3D12RHI& gfx, MeshBindables& meshBindables)
	//{
	//	rootSignBindable = std::move(meshBindables.rootSignBindablePtr);
	//	psoBindable = std::move(meshBindables.psoBindablePtr);
	//	if (meshBindables.srvBindablePtr != nullptr) srvBindable = std::move(meshBindables.srvBindablePtr);

	//	topologyBindable = std::move(meshBindables.topologyBindablePtr);
	//	vertexBufferBindable = std::move(meshBindables.vertexBufferBindablePtr);
	//	indexBufferBindable = std::move(meshBindables.indexBufferBindablePtr);

	//	for (auto& pb : meshBindables.bindablePtrs)
	//	{
	//		AddBindable(std::move(pb));
	//	}
	//}

	void Mesh::Submit(DirectX::FXMMATRIX accumulatedTranform) const noexcept
	{
		XMStoreFloat4x4(&transform, accumulatedTranform);
		Drawable::Submit();
	}

	Mesh::Mesh(D3D12RHI& gfx, Material& mat, const aiMesh& mesh, float scale) noexcept
		:
	Drawable(gfx, mat, mesh, scale)
	{
		enableLighting = true;
	}

	void Mesh::SetNumIndices(UINT numIndices)
	{
		m_numIndices = numIndices;
	}

	XMMATRIX Mesh::GetTransformXM() const noexcept
	{
		return XMLoadFloat4x4(&transform);
	}
}