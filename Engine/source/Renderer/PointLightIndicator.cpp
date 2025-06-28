#include "PointLightIndicator.h"

namespace Renderer
{
	PointLightIndicator::PointLightIndicator(D3D12RHI& gfx, float radius)
	{
		m_indexedList = Sphere::Make();		
		m_indexedList.Transform(XMMatrixScaling(radius, radius, radius)); // deform vertices of model by linear transformation

		SolidCB data = { XMFLOAT3{ 1.0f,1.0f,1.0f } };
		m_solidMatHandles.solidConstIdx = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &data, sizeof(data)));
		m_materialIdx = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &m_solidMatHandles, sizeof(m_solidMatHandles)));

		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices);
	}

	void PointLightIndicator::Update(XMFLOAT3 pos) noexcept(!IS_DEBUG)
	{
		m_transforms.meshMat = XMMatrixTranslation(pos.x, pos.y, pos.z);
		m_transforms.meshInvMat = XMMatrixInverse(nullptr, m_transforms.meshMat);
	}
}