#include "CameraIndicator.h"

namespace Renderer
{
	CameraIndicator::CameraIndicator(D3D12RHI& gfx)
	{
		m_meshIdx = ++m_meshCount;
		m_indexedList = Pyramid::Make();

		auto material = std::make_shared<LineMaterial>(gfx, m_indexedList.vertices.GetLayout());
		m_materialTypeId = material->getID();

		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices, material->GetTopology());
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

	void CameraIndicator::SetTransform(D3D12RHI& gfx, std::string targetPass) const noexcept(!IS_DEBUG)
	{
		const auto model = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_rot)) * XMMatrixTranslationFromVector(XMLoadFloat3(&m_pos));
		const auto modelView = model * m_cameraMatrix;

		/*
		* Convert all XMMATRIX or XMFLOAT4X4 which are Row - major into Column - major matrix which is used by HLSL by default.
		* Use XMMatrixTranspose() to achieve this.
		*/
		m_transforms = {
			XMMatrixTranspose(model),
			XMMatrixTranspose(modelView),
			XMMatrixTranspose(
				modelView *
				m_projectionMatrix
			)
		};

		m_meshConstants = { m_materialTypeId, m_materialIdx };

		gfx.Set32BitRootConstants(0, sizeof(m_transforms) / 4, &m_transforms);
		gfx.Set32BitRootConstants(1, 2, &m_meshConstants);
	}
}