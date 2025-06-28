#include "LightContainer.h"

namespace Renderer
{
	LightContainer::LightContainer(D3D12RHI& gfx, CameraContainer& cameraContainer)
		:
		m_cameraContainer(cameraContainer)
	{

	}

	bool LightContainer::SpawnWindow(D3D12RHI& gfx)
	{
		if (ImGui::Begin("Lights", &m_imGUIwndOpen))
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

			GetControlledLight().SpawnControlWidgets(gfx);
		}
		ImGui::End();

		return m_imGUIwndOpen;
	}

	void LightContainer::AddLight(std::shared_ptr<Light> plight)
	{
		m_lights.push_back(std::move(plight));
		m_lightBufferData.push_back(m_lights.back()->GetLightData());
	}

	void LightContainer::UpdateLights(D3D12RHI& gfx)
	{
		auto cameraViewMat = m_cameraContainer.GetActiveCamera().GetViewMatrix();

		for (size_t i = 0; i < m_lightBufferData.size(); i++)
		{
			m_lights[i]->Update(gfx, cameraViewMat);
			m_lightBufferData[i] = m_lights[i]->GetLightData();
		}

		if (!m_lightBuffer)
		{
			m_lightBuffer = std::make_shared<D3D12Buffer>(gfx, m_lightBufferData.data(), sizeof(LightData) * m_lightBufferData.size(), sizeof(LightData));
			RenderGraph::m_frameData.lightDataHandle = gfx.LoadResource(m_lightBuffer, D3D12Resource::ViewType::SRV);
		}
		else
		{
			m_lightBuffer->Update(gfx, m_lightBufferData.data(), sizeof(LightData) * m_lightBufferData.size(), BufferType::Constant);
		}		
	}
}