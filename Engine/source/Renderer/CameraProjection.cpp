#include "CameraProjection.h"

namespace Renderer
{
	CameraProjection::CameraProjection(D3D12RHI& gfx, float width, float height, float nearZ, float farZ)
	{
		m_indexedList = Frustum::Make(width, height, nearZ, farZ);

		m_drawData.vertices = m_indexedList.vertices;
		m_drawData.indices = m_indexedList.indices;
		m_drawData.vertexSizeInBytes = m_drawData.vertices.size() * sizeof(m_drawData.vertices[0]);
		m_drawData.indexSizeInBytes = m_drawData.indices.size() * sizeof(m_drawData.indices[0]);
		m_drawData.vertexStrideInBytes = sizeof(VertexStruct);

		m_drawData.vertexBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, m_drawData.vertices.data(), m_drawData.vertexSizeInBytes));
		m_drawData.indexBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, m_drawData.indices.data(), m_drawData.indexSizeInBytes));
	}

	void CameraProjection::SetVertices(float width, float height, float nearZ, float farZ)
	{
		m_indexedList = Frustum::Make(width, height, nearZ, farZ);

		m_drawData.vertexBuffer->UpdateGPU(m_indexedList.vertices.data(), m_indexedList.vertices.size() * sizeof(m_indexedList.vertices[0]), 0);
	}

	void CameraProjection::Update(Vector3 position, Vector3 rotation) noexcept(!IS_DEBUG)
	{
		m_transforms.meshMat = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation)) * XMMatrixTranslationFromVector(XMLoadFloat3(&position));
		m_transforms.meshInvMat = XMMatrixInverse(nullptr, m_transforms.meshMat);
	}
}