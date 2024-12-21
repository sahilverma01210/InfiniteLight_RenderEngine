#include "Skybox.h"

namespace Renderer
{
	Skybox::Skybox(D3D12RHI& gfx)
	{
		m_indexedList = Cube::Make();
		m_indexedList.Transform(XMMatrixScaling(3.0f, 3.0f, 3.0f));

		SkyboxMaterial material(gfx, m_indexedList.vertices.GetLayout());

		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices);
		ApplyMaterial(gfx, material);
	}

	XMMATRIX Skybox::GetTransformXM() const noexcept(!IS_DEBUG)
	{
		return XMMATRIX();
	}
}