#include "Skybox.h"

namespace Renderer
{
	Skybox::Skybox(D3D12RHI& gfx)
	{
		m_meshIdx = ++m_meshCount;

		m_indexedList = Cube::Make();
		m_indexedList.Transform(XMMatrixScaling(3.0f, 3.0f, 3.0f));

		auto material = std::make_shared<SkyboxMaterial>(gfx, m_indexedList.vertices.GetLayout());

		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices);
		ApplyMaterial(gfx, material.get());
	}

	void Skybox::SetTransform(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		Transforms transforms{};
		transforms.meshMat = XMMatrixIdentity();
		transforms.meshInvMat = XMMatrixInverse(nullptr, transforms.meshMat);

		gfx.Set32BitRootConstants(1, sizeof(transforms) / 4, &transforms);
		gfx.Set32BitRootConstants(2, 1, &m_materialIdx);
	}
}