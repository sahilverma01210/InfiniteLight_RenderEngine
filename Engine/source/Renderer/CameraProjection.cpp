#include "CameraProjection.h"

namespace Renderer
{
	CameraProjection::CameraProjection(D3D12RHI& gfx, float width, float height, float nearZ, float farZ)
	{
		m_indexedList = Frustum::Make(width, height, nearZ, farZ);

		SolidCB solidCB = { XMFLOAT3{ 0.6f,0.2f,0.2f } };
		m_lineWireMatHandles.solidConstIdx = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &solidCB, sizeof(solidCB)));
		m_materialIdx = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &m_lineWireMatHandles, sizeof(m_lineWireMatHandles)));
		
		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices);
	}

	void CameraProjection::SetVertices(D3D12RHI& gfx, float width, float height, float nearZ, float farZ)
	{
		m_indexedList = Frustum::Make(width, height, nearZ, farZ);

		m_drawData.vertexBuffer->Update(gfx, m_indexedList.vertices.GetData(), UINT(m_indexedList.vertices.SizeBytes()), BufferType::Vertex);
	}

	void CameraProjection::Update(Vector3 position, Vector3 rotation) noexcept(!IS_DEBUG)
	{
		m_transforms.meshMat = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation)) * XMMatrixTranslationFromVector(XMLoadFloat3(&position));
		m_transforms.meshInvMat = XMMatrixInverse(nullptr, m_transforms.meshMat);
	}
}