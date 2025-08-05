#include "CameraIndicator.h"

namespace Renderer
{
	CameraIndicator::CameraIndicator(D3D12RHI& gfx)
	{
		m_indexedList = Pyramid::Make();

		m_drawData.vertices = m_indexedList.vertices;
		m_drawData.indices = m_indexedList.indices;
		m_drawData.vertexSizeInBytes = m_drawData.vertices.size() * sizeof(m_drawData.vertices[0]);
		m_drawData.indexSizeInBytes = m_drawData.indices.size() * sizeof(m_drawData.indices[0]);
		m_drawData.vertexStrideInBytes = sizeof(VertexStruct);

		m_drawData.vertexBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, m_drawData.vertices.data(), m_drawData.vertexSizeInBytes));
		m_drawData.indexBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, m_drawData.indices.data(), m_drawData.indexSizeInBytes));
	}

	void CameraIndicator::Update(XMFLOAT3 position, XMFLOAT3 rotation) noexcept(!IS_DEBUG)
	{
		m_transforms.meshMat = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation)) * XMMatrixTranslationFromVector(XMLoadFloat3(&position));
		m_transforms.meshInvMat = XMMatrixInverse(nullptr, m_transforms.meshMat);
	}
}