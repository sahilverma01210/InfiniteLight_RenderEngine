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

	void Skybox::SetTransform(D3D12RHI& gfx, std::string targetPass) const noexcept(!IS_DEBUG)
	{
		m_transforms = {
			XMMatrixTranspose(m_cameraMatrix * m_projectionMatrix)
		};

		gfx.Set32BitRootConstants(0, sizeof(m_transforms) / 4, &m_transforms);
	}
}