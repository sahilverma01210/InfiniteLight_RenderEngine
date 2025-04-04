#include "CameraProjection.h"

namespace Renderer
{
	CameraProjection::CameraProjection(D3D12RHI& gfx, Projection projection)
	{
		m_meshIdx = ++m_meshCount;
		m_indexedList = Frustum::Make(projection.width, projection.height, projection.nearZ, projection.farZ);

		auto material = std::make_shared<LineWireMaterial>(gfx, m_indexedList.vertices.GetLayout());
		m_materialTypeId = material->getID();

		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices, material->GetTopology());
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