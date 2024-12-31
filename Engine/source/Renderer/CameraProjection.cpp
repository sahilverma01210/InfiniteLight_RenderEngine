#include "CameraProjection.h"

namespace Renderer
{
	CameraProjection::CameraProjection(D3D12RHI& gfx, Projection projection)
		:
		m_homeProjection(projection)
	{
		m_indexedList = Frustum::Make(m_homeProjection.width, m_homeProjection.height, m_homeProjection.nearZ, m_homeProjection.farZ);

		auto material = std::make_shared<LineWireMaterial>(gfx, m_indexedList.vertices.GetLayout());

		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices, material->GetTopology());
		ApplyMaterial(gfx, material.get());

		m_projection = m_homeProjection;
	}

	void CameraProjection::RenderWidgets(D3D12RHI& gfx)
	{
		bool dirty = false;
		const auto dcheck = [&dirty](bool d) { dirty = dirty || d; };
		ImGui::Text("Projection");
		dcheck(ImGui::SliderFloat("Width", &m_projection.width, 0.01f, 4.0f, "%.2f"));
		dcheck(ImGui::SliderFloat("Height", &m_projection.height, 0.01f, 4.0f, "%.2f"));
		dcheck(ImGui::SliderFloat("Near Z", &m_projection.nearZ, 0.01f, m_projection.farZ - 0.01f, "%.2f"));
		dcheck(ImGui::SliderFloat("Far Z", &m_projection.farZ, m_projection.nearZ + 0.01f, 400.0f, "%.2f"));
		if (dirty)
		{
			SetVertices(gfx, m_projection);
		}
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

	XMMATRIX CameraProjection::GetProjectionMatrix() const
	{
		return XMMatrixPerspectiveLH(m_projection.width, m_projection.height, m_projection.nearZ, m_projection.farZ);
	}

	void CameraProjection::Reset(D3D12RHI& gfx)
	{
		m_projection = m_homeProjection;

		SetVertices(gfx, m_projection);
	}

	void CameraProjection::SetTransform(D3D12RHI& gfx, std::string targetPass) const noexcept(!IS_DEBUG)
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

		gfx.Set32BitRootConstants(0, sizeof(m_transforms) / 4, &m_transforms);
	}
}