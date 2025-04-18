#include "PointLightIndicator.h"

namespace Renderer
{
	PointLightIndicator::PointLightIndicator(D3D12RHI& gfx, float radius)
	{
		m_meshIdx = ++m_meshCount;

		m_indexedList = Sphere::Make();		
		m_indexedList.Transform(XMMatrixScaling(radius, radius, radius)); // deform vertices of model by linear transformation

		auto material = std::make_shared<SolidMaterial>(gfx, m_indexedList.vertices.GetLayout(), XMFLOAT3{ 1.0f,1.0f,1.0f });
		m_materialTypeId = material->getID();

		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices);
		ApplyMaterial(gfx, material.get());
	}

	void PointLightIndicator::SetPos(XMFLOAT3 pos) noexcept(!IS_DEBUG)
	{
		this->m_pos = pos;
	}

	void PointLightIndicator::SetTransform(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		const auto model = XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);
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