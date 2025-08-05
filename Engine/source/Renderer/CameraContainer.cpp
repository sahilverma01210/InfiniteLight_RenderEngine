#include "CameraContainer.h"

namespace Renderer
{
	CameraContainer::CameraContainer(D3D12RHI& gfx) : m_gfx(gfx)
	{
		m_cameraBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, &m_cameraBufferData, sizeof(m_cameraBufferData), sizeof(Camera::CameraData)));
		RenderGraph::m_frameData.cameraDataHandle = gfx.LoadResource(m_cameraBuffer, D3D12Resource::ViewType::SRV);
	}

	bool CameraContainer::SpawnWindow()
	{
		if (ImGui::Begin("Cameras", &m_imGUIwndOpen, ImGuiWindowFlags_NoBackground))
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

			GetControlledCamera().SpawnControlWidgets();
		}
		ImGui::End();

		return m_imGUIwndOpen;
	}

	void CameraContainer::AddCamera(std::shared_ptr<Camera> pCam)
	{
		pCam->SetInvertProjection(m_gfx.IsRayTracingEnabled());

		m_cameras.push_back(std::move(pCam));
	}

	bool CameraContainer::UpdateCamera()
	{
		GetActiveCamera().Update();

		m_cameraBufferData = GetActiveCamera().GetCameraData();

		m_cameraBuffer->UpdateGPU(&m_cameraBufferData, sizeof(m_cameraBufferData), 0);

		auto& cameraState = GetActiveCamera().GetState();

		return (cameraState.move || cameraState.rotate);
	}
}