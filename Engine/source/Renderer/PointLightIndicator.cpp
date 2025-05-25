#include "PointLightIndicator.h"

namespace Renderer
{
	PointLightIndicator::PointLightIndicator(D3D12RHI& gfx, float radius)
	{
		m_meshIdx = ++m_meshCount;

		m_indexedList = Sphere::Make();		
		m_indexedList.Transform(XMMatrixScaling(radius, radius, radius)); // deform vertices of model by linear transformation

		auto material = std::make_shared<SolidMaterial>(gfx, m_indexedList.vertices.GetLayout(), XMFLOAT3{ 1.0f,1.0f,1.0f });
		
		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices);
		ApplyMaterial(gfx, material.get());
	}

	void PointLightIndicator::SetPos(XMFLOAT3 pos) noexcept(!IS_DEBUG)
	{
		this->m_pos = pos;
	}

	void PointLightIndicator::SetTransform(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		Transforms transforms{};
		transforms.meshMat = XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);
		transforms.meshInvMat = XMMatrixInverse(nullptr, transforms.meshMat);

		gfx.Set32BitRootConstants(1, sizeof(transforms) / 4, &transforms);
		gfx.Set32BitRootConstants(2, 1, &m_materialIdx);
	}
}