#include "CameraProjection.h"

namespace Renderer
{
	CameraProjection::CameraProjection(D3D12RHI& gfx, float width, float height, float nearZ, float farZ)
		:
		m_homeWidth(width), m_homeHeight(height), m_homeNearZ(nearZ), m_homeFarZ(farZ)
	{
		m_indexedList = Frustum::Make(width, height, nearZ, farZ);

		LineWireMaterial material(gfx, m_indexedList.vertices.GetLayout());

		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices, material.GetTopology());
		ApplyMaterial(gfx, material);

		m_width = m_homeWidth;
		m_height = m_homeHeight;
		m_nearZ = m_homeNearZ;
		m_farZ = m_homeFarZ;
	}

	void CameraProjection::RenderWidgets(D3D12RHI& gfx)
	{
		bool dirty = false;
		const auto dcheck = [&dirty](bool d) { dirty = dirty || d; };
		ImGui::Text("Projection");
		dcheck(ImGui::SliderFloat("Width", &m_width, 0.01f, 4.0f, "%.2f"));
		dcheck(ImGui::SliderFloat("Height", &m_height, 0.01f, 4.0f, "%.2f"));
		dcheck(ImGui::SliderFloat("Near Z", &m_nearZ, 0.01f, m_farZ - 0.01f, "%.2f"));
		dcheck(ImGui::SliderFloat("Far Z", &m_farZ, m_nearZ + 0.01f, 400.0f, "%.2f"));
		if (dirty)
		{
			SetVertices(gfx, m_width, m_height, m_nearZ, m_farZ);
		}
	}

	void CameraProjection::SetVertices(D3D12RHI& gfx, float width, float height, float nearZ, float farZ)
	{
		m_indexedList = Frustum::Make(width, height, nearZ, farZ);
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

	XMMATRIX CameraProjection::GetMatrix() const
	{
		return XMMatrixPerspectiveLH(m_width, m_height, m_nearZ, m_farZ);
	}

	XMMATRIX CameraProjection::GetTransformXM() const noexcept(!IS_DEBUG)
	{
		return XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_rot)) *
			XMMatrixTranslationFromVector(XMLoadFloat3(&m_pos));
	}

	void CameraProjection::Reset(D3D12RHI& gfx)
	{
		m_width = m_homeWidth;
		m_height = m_homeHeight;
		m_nearZ = m_homeNearZ;
		m_farZ = m_homeFarZ;
		SetVertices(gfx, m_width, m_height, m_nearZ, m_farZ);
	}
}