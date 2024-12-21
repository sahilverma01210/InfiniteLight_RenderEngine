#include "CameraContainer.h"

namespace Renderer
{
	CameraContainer::~CameraContainer()
	{
	}

	bool CameraContainer::SpawnWindow(D3D12RHI& gfx)
	{
		if (ImGui::Begin("Cameras", &m_imGUIwndOpen))
		{
			if (ImGui::BeginCombo("Active Camera", GetActiveCamera().GetName().c_str()))
			{
				for (int i = 0; i < std::size(m_cameras); i++)
				{
					const bool isSelected = i == m_active;
					if (ImGui::Selectable(m_cameras[i]->GetName().c_str(), isSelected))
					{
						m_active = i;
					}
				}
				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo("Controlled Camera", GetControlledCamera().GetName().c_str()))
			{
				for (int i = 0; i < std::size(m_cameras); i++)
				{
					const bool isSelected = i == m_controlled;
					if (ImGui::Selectable(m_cameras[i]->GetName().c_str(), isSelected))
					{
						m_controlled = i;
					}
				}
				ImGui::EndCombo();
			}

			GetControlledCamera().SpawnControlWidgets(gfx);
		}
		ImGui::End();

		return m_imGUIwndOpen;
	}

	void CameraContainer::Bind(D3D12RHI& gfx)
	{
		gfx.SetCamera(GetActiveCamera().GetMatrix());
	}

	void CameraContainer::AddCamera(std::shared_ptr<Camera> pCam)
	{
		m_cameras.push_back(std::move(pCam));
	}

	void CameraContainer::LinkTechniques(RenderGraph& rg)
	{
		for (auto& pcam : m_cameras)
		{
			pcam->LinkTechniques(rg);
		}
	}

	void CameraContainer::Submit(size_t channel) const
	{
		for (size_t i = 0; i < m_cameras.size(); i++)
		{
			if (i != m_active)
			{
				m_cameras[i]->Submit(channel);
			}
		}
	}

	Camera& CameraContainer::GetActiveCamera()
	{
		return *m_cameras[m_active];
	}

	Camera& CameraContainer::GetControlledCamera()
	{
		return *m_cameras[m_controlled];
	}
}