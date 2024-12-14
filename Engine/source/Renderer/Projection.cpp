#include "Projection.h"

namespace Renderer
{
	Projection::Projection(D3D12RHI& gfx, float width, float height, float nearZ, float farZ)
		:
		m_width(width),
		m_height(height),
		m_nearZ(nearZ),
		m_farZ(farZ),
		m_frust(gfx, width, height, nearZ, farZ),
		m_homeWidth(width), m_homeHeight(height), m_homeNearZ(nearZ), m_homeFarZ(farZ)
	{}

	XMMATRIX Projection::GetMatrix() const
	{
		return XMMatrixPerspectiveLH(m_width, m_height, m_nearZ, m_farZ);
	}

	void Projection::RenderWidgets(D3D12RHI& gfx)
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
			m_frust.SetVertices(gfx, m_width, m_height, m_nearZ, m_farZ);
		}
	}

	void Projection::SetPos(XMFLOAT3 pos)
	{
		m_frust.SetPos(pos);
	}

	void Projection::SetRotation(XMFLOAT3 rot)
	{
		m_frust.SetRotation(rot);
	}

	void Projection::Submit(size_t channel) const
	{
		m_frust.Submit(channel);
	}

	void Projection::LinkTechniques(RenderGraph& rg)
	{
		m_frust.LinkTechniques(rg);
	}

	void Projection::Reset(D3D12RHI& gfx)
	{
		m_width = m_homeWidth;
		m_height = m_homeHeight;
		m_nearZ = m_homeNearZ;
		m_farZ = m_homeFarZ;
		m_frust.SetVertices(gfx, m_width, m_height, m_nearZ, m_farZ);
	}
}