#include "CameraContainer.h"

namespace Renderer
{
	CameraContainer::CameraContainer(D3D12RHI& gfx)
	{
		m_CameraConstants = std::make_shared<ConstantBuffer>(gfx, sizeof(m_cameraCBuf), &m_cameraCBuf);
		RenderPass::m_cameraDataHandle = gfx.LoadResource(m_CameraConstants, ResourceType::Constant);
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

	void CameraContainer::AddCamera(std::shared_ptr<Camera> pCam)
	{
		m_cameras.push_back(std::move(pCam));
	}

	void CameraContainer::UpdateCamera(D3D12RHI& gfx, CameraCBuf* cameraCBuf)
	{
		GetActiveCamera().SetViewMatrix();
		GetActiveCamera().SetProjectionMatrix();

		m_cameraCBuf.viewMat = GetActiveCamera().GetViewMatrix();
		m_cameraCBuf.projectionMat = GetActiveCamera().GetProjectionMatrix();
		m_cameraCBuf.inverseViewMat = m_cameraCBuf.viewMat.Invert();
		m_cameraCBuf.inverseProjectionMat = m_cameraCBuf.projectionMat.Invert();

		m_CameraConstants->Update(gfx, cameraCBuf ? cameraCBuf : &m_cameraCBuf);
	}

	void CameraContainer::Submit(RenderGraph& renderGraph) const
	{
		for (size_t i = 0; i < m_cameras.size(); i++)
		{
			if (i != m_active)
			{
				m_cameras[i]->Submit(renderGraph);
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