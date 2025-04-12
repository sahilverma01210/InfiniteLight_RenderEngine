#include "PointLight.h"

namespace Renderer
{
	PointLight::PointLight(D3D12RHI& gfx, PointLightCBuf& home, CameraContainer& cameraContainer, float radius)
		:
		m_gfx(gfx),
		m_home(home),
		m_cameraContainer(cameraContainer),
		m_indicator(gfx, radius)
	{
		m_cbData = m_home;

		m_lightConstants = std::make_shared<ConstantBuffer>(gfx, sizeof(m_cbData), &m_cbData);
		ILMaterial::m_lightHandle = gfx.LoadResource(m_lightConstants, ResourceType::Constant);
	}

	bool PointLight::SpawnWindow() noexcept(!IS_DEBUG)
	{
		if (ImGui::Begin("Light", &m_imGUIwndOpen))
		{
			bool dirtyPos = false;
			const auto d = [&dirtyPos](bool dirty) {dirtyPos = dirtyPos || dirty; };

			ImGui::Text("Position");
			d(ImGui::SliderFloat("X", &m_cbData.pos.x, -60.0f, 60.0f, "%.1f"));
			d(ImGui::SliderFloat("Y", &m_cbData.pos.y, -60.0f, 60.0f, "%.1f"));
			d(ImGui::SliderFloat("Z", &m_cbData.pos.z, -60.0f, 60.0f, "%.1f"));
			ImGui::Text("Intensity/Color");
			ImGui::SliderFloat("Intensity", &m_cbData.diffuseIntensity, 0.01f, 2.0f, "%.2f");
			ImGui::ColorEdit3("Diffuse Color", &m_cbData.diffuseColor.x);
			ImGui::ColorEdit3("Ambient", &m_cbData.ambient.x);

			ImGui::Text("Falloff");
			ImGui::SliderFloat("Constant", &m_cbData.attConst, 0.05f, 10.0f, "%.2f");
			ImGui::SliderFloat("Linear", &m_cbData.attLin, 0.0001f, 4.0f, "%.4f");
			ImGui::SliderFloat("Quadratic", &m_cbData.attQuad, 0.0000001f, 10.0f, "%.7f");

			if (ImGui::Button("Reset"))
			{
				Reset();
			}
		}
		ImGui::End();

		return m_imGUIwndOpen;
	}

	void PointLight::Reset() noexcept(!IS_DEBUG)
	{
		m_cbData = m_home;
	}

	void PointLight::Submit(RenderGraph& renderGraph) const noexcept(!IS_DEBUG)
	{
		m_indicator.SetPos(m_cbData.pos);
		m_indicator.Submit(renderGraph);
	}

	void PointLight::Update(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		ILMaterial::m_lightPosition = m_cbData.pos;

		auto dataCopy = m_cbData;
		const auto pos = XMLoadFloat3(&m_cbData.pos);
		XMStoreFloat3(&dataCopy.viewPos, XMVector3Transform(pos, m_cameraContainer.GetActiveCamera().GetCameraMatrix()));

		m_lightConstants->Update(gfx, &dataCopy);
	}
}