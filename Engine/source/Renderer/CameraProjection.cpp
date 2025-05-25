#include "CameraProjection.h"

namespace Renderer
{
	CameraProjection::CameraProjection(D3D12RHI& gfx, Projection projection)
	{
		m_meshIdx = ++m_meshCount;
		m_indexedList = Frustum::Make(projection.width, projection.height, projection.nearZ, projection.farZ);

		auto material = std::make_shared<WireframeMaterial>(gfx, m_indexedList.vertices.GetLayout(), XMFLOAT3{ 0.6f,0.2f,0.2f });
		
		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices);
		ApplyMaterial(gfx, material.get());
	}

	void CameraProjection::SetVertices(D3D12RHI& gfx, Projection projection)
	{
		m_indexedList = Frustum::Make(projection.width, projection.height, projection.nearZ, projection.farZ);

		m_vertexBufferBindable->Update(gfx, m_indexedList.vertices.GetData());
	}

	void CameraProjection::SetPos(XMFLOAT3 pos)
	{
		this->m_pos = pos;
	}

	void CameraProjection::SetRotation(XMFLOAT3 rot)
	{
		this->m_rot = rot;
	}

	void CameraProjection::SetTransform(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		Transforms transforms{};
		transforms.meshMat = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_rot)) * XMMatrixTranslationFromVector(XMLoadFloat3(&m_pos));
		transforms.meshInvMat = XMMatrixInverse(nullptr, transforms.meshMat);

		gfx.Set32BitRootConstants(1, sizeof(transforms) / 4, &transforms);
		gfx.Set32BitRootConstants(2, 1, &m_materialIdx);
	}
}