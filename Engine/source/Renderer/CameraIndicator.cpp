#include "CameraIndicator.h"

namespace Renderer
{
	CameraIndicator::CameraIndicator(D3D12RHI& gfx)
	{
		m_indexedList = Pyramid::Make();

		SolidCB data = { XMFLOAT3{ 0.0f,0.0f,1.0f } };
		m_solidMatHandles.solidConstIdx = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &data, sizeof(data)));
		m_materialIdx = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &m_solidMatHandles, sizeof(m_solidMatHandles)));

		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices);
	}

	void CameraIndicator::Update(XMFLOAT3 position, XMFLOAT3 rotation) noexcept(!IS_DEBUG)
	{
		m_transforms.meshMat = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation)) * XMMatrixTranslationFromVector(XMLoadFloat3(&position));
		m_transforms.meshInvMat = XMMatrixInverse(nullptr, m_transforms.meshMat);
	}
}