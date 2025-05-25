#include "CameraIndicator.h"

namespace Renderer
{
	CameraIndicator::CameraIndicator(D3D12RHI& gfx)
	{
		m_meshIdx = ++m_meshCount;
		m_indexedList = Pyramid::Make();

		auto material = std::make_shared<SolidMaterial>(gfx, m_indexedList.vertices.GetLayout(), XMFLOAT3{ 0.0f,0.0f,1.0f });
		
		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices);
		ApplyMaterial(gfx, material.get());
	}

	void CameraIndicator::SetPos(XMFLOAT3 pos) noexcept(!IS_DEBUG)
	{
		this->m_pos = pos;
	}

	void CameraIndicator::SetRotation(XMFLOAT3 rot) noexcept(!IS_DEBUG)
	{
		this->m_rot = rot;
	}

	void CameraIndicator::SetTransform(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		Transforms transforms{};
		transforms.meshMat = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_rot)) * XMMatrixTranslationFromVector(XMLoadFloat3(&m_pos));
		transforms.meshInvMat = XMMatrixInverse(nullptr, transforms.meshMat);

		gfx.Set32BitRootConstants(1, sizeof(transforms) / 4, &transforms);
		gfx.Set32BitRootConstants(2, 1, &m_materialIdx);
	}
}