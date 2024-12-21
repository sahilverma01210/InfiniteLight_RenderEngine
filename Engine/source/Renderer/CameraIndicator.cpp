#include "CameraIndicator.h"

namespace Renderer
{
	CameraIndicator::CameraIndicator(D3D12RHI& gfx)
	{
		m_indexedList = Pyramid::Make();

		LineMaterial material(gfx, m_indexedList.vertices.GetLayout());

		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices, material.GetTopology());
		ApplyMaterial(gfx, material);
	}

	void CameraIndicator::SetPos(XMFLOAT3 pos) noexcept(!IS_DEBUG)
	{
		this->m_pos = pos;
	}

	void CameraIndicator::SetRotation(XMFLOAT3 rot) noexcept(!IS_DEBUG)
	{
		this->m_rot = rot;
	}

	XMMATRIX CameraIndicator::GetTransformXM() const noexcept(!IS_DEBUG)
	{
		return XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_rot)) *
			XMMatrixTranslationFromVector(XMLoadFloat3(&m_pos));
	}
}