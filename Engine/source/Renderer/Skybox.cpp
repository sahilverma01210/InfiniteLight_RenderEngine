#include "Skybox.h"

namespace Renderer
{
	Skybox::Skybox(D3D12RHI& gfx)
	{
		m_meshIdx = ++m_meshCount;

		m_indexedList = Cube::Make();
		m_indexedList.Transform(XMMatrixScaling(3.0f, 3.0f, 3.0f));

		auto material = std::make_shared<SkyboxMaterial>(gfx, m_indexedList.vertices.GetLayout());
		m_materialTypeId = material->getID();

		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices);
		ApplyMaterial(gfx, material.get());
	}

	void Skybox::SetTransform(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		m_transforms = {
			XMMatrixIdentity(),
			XMMatrixIdentity(),
			XMMatrixTranspose(m_cameraMatrix * m_projectionMatrix)
		};

		m_meshConstants = { m_materialTypeId, m_materialIdx };

		gfx.Set32BitRootConstants(0, sizeof(m_transforms) / 4, &m_transforms);
		gfx.Set32BitRootConstants(1, 2, &m_meshConstants);
	}
}