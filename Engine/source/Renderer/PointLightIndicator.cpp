#include "PointLightIndicator.h"

namespace Renderer
{
	PointLightIndicator::PointLightIndicator(D3D12RHI& gfx, float radius)
	{
		m_indexedList = Sphere::Make();		
		m_indexedList.Transform(XMMatrixScaling(radius, radius, radius)); // deform vertices of model by linear transformation

		SolidMaterial material(gfx, m_indexedList.vertices.GetLayout());

		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices);
		ApplyMaterial(gfx, material);
	}

	void PointLightIndicator::SetPos(XMFLOAT3 pos) noexcept(!IS_DEBUG)
	{
		this->m_pos = pos;
	}

	XMMATRIX PointLightIndicator::GetTransformXM() const noexcept(!IS_DEBUG)
	{
		return XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);
	}
}