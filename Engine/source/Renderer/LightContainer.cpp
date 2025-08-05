#include "LightContainer.h"

namespace Renderer
{
	LightContainer::LightContainer(D3D12RHI& gfx, CameraContainer& cameraContainer)
		:
		m_gfx(gfx),
		m_cameraContainer(cameraContainer)
	{

	}

	bool LightContainer::SpawnWindow()
	{
		if (ImGui::Begin("Lights", &m_imGUIwndOpen, ImGuiWindowFlags_NoBackground))
		{
			if (ImGui::BeginCombo("Controlled Light", GetControlledLight().GetName().c_str()))
			{
				for (int i = 0; i < std::size(m_lights); i++)
				{
					const bool isSelected = i == m_controlled;
					if (ImGui::Selectable(m_lights[i]->GetName().c_str(), isSelected))
					{
						m_controlled = i;
					}
				}
				ImGui::EndCombo();
			}

			GetControlledLight().SpawnControlWidgets();
		}
		ImGui::End();

		return m_imGUIwndOpen;
	}

	void LightContainer::AddLight(std::shared_ptr<Light> plight)
	{
		m_lights.push_back(std::move(plight));
		m_lightBufferData.push_back(m_lights.back()->GetLightData());
	}

	void LightContainer::UpdateLights()
	{
		auto cameraViewMat = m_cameraContainer.GetActiveCamera().GetViewMatrix();

		for (size_t i = 0; i < m_lightBufferData.size(); i++)
		{
			m_lights[i]->Update(cameraViewMat);
			m_lightBufferData[i] = m_lights[i]->GetLightData();
		}

		if (!m_lightBuffer)
		{
			m_lightBuffer = std::move(std::make_shared<D3D12Buffer>(m_gfx, m_lightBufferData.data(), sizeof(LightData) * m_lightBufferData.size(), sizeof(LightData)));
			RenderGraph::m_frameData.lightDataHandle = m_gfx.LoadResource(m_lightBuffer, D3D12Resource::ViewType::SRV);
			RenderGraph::m_frameData.lightCount = m_lightBufferData.size();
		}
		else
		{
			m_lightBuffer->UpdateGPU(m_lightBufferData.data(), sizeof(LightData) * m_lightBufferData.size(), 0);
		}		
	}
}