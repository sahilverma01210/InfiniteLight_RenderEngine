#include "PointLight.h"

namespace Renderer
{
	PointLight::PointLight(D3D12RHI& gfx, std::string name, LightData& home, float radius)
		:
		Light(name),
		m_gfx(gfx),
		m_home(home),
		m_radius(radius)
	{
		m_cbData = m_home;

		//m_lightConstants = std::make_shared<D3D12Buffer>(gfx, &m_cbData, sizeof(m_cbData), sizeof(LightData));
		//RenderGraph::m_frameData.lightDataHandle = gfx.LoadResource(m_lightConstants, D3D12Resource::ViewType::SRV);
	}

	void PointLight::SpawnControlWidgets(D3D12RHI& gfx) noexcept(!IS_DEBUG)
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

		if (ImGui::Button("Reset"))
		{
			Reset();
		}
	}

	void PointLight::Reset() noexcept(!IS_DEBUG)
	{
		m_cbData = m_home;
	}

	void PointLight::Update(D3D12RHI& gfx, Matrix& viewMat) const noexcept(!IS_DEBUG)
	{
		m_cbData.shadowDepthIdx = m_shadowMapHandle;
		XMStoreFloat3(&m_cbData.viewPos, XMVector3Transform(XMLoadFloat3(&m_cbData.pos), viewMat));

		//m_lightConstants->Update(gfx, &m_cbData, sizeof(m_cbData), BufferType::Constant);
	}
}