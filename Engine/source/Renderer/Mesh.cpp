#include "Mesh.h"

namespace Renderer
{
	Mesh::Mesh(D3D12RHI& gfx, Material& mat, const aiMesh& mesh, float scale) noexcept(!IS_DEBUG)
		:
	Drawable(gfx, mat, mesh, scale)
	{
		enableLighting = true;
	}

	void Mesh::Submit(size_t channels, FXMMATRIX accumulatedTranform) const noexcept(!IS_DEBUG)
	{
		XMStoreFloat4x4(&transform, accumulatedTranform);
		Drawable::Submit(channels);
	}

	void Mesh::SetNumIndices(UINT numIndices)
	{
		m_numIndices = numIndices;
	}

	XMMATRIX Mesh::GetTransformXM() const noexcept(!IS_DEBUG)
	{
		return XMLoadFloat4x4(&transform);
	}
}