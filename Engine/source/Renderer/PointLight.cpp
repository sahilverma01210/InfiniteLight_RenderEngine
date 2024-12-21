#include "PointLight.h"

namespace Renderer
{
	PointLight::PointLight(D3D12RHI& gfx, XMFLOAT3 pos, float radius)
		:
		m_indicator(gfx, radius)
	{
		m_home = {
			pos,
			{ 0.05f,0.05f,0.05f },
			{ 1.0f,1.0f,1.0f },
			1.0f,
			1.0f,
			0.025f,
			0.0030f,
		};

		Reset();
		m_pCamera = std::make_shared<Camera>(gfx, "Light", m_cbData.pos, 0.0f, PI / 2.0f, true);
		Drawable::m_lightShadowBindable = std::move(std::make_unique<ConstantBuffer>(gfx, 1, sizeof(m_shadowData), &m_shadowData));
		Drawable::m_lightBindable = std::move(std::make_unique<ConstantBuffer>(gfx, 2, sizeof(m_cbData), &m_cbData));
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
			if (dirtyPos)
			{
				m_pCamera->SetPos(m_cbData.pos);
			}
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

	void PointLight::Submit(size_t channels) const noexcept(!IS_DEBUG)
	{
		m_indicator.SetPos(m_cbData.pos);
		m_indicator.Submit(channels);
	}

	void PointLight::Update(D3D12RHI& gfx, FXMMATRIX view) const noexcept(!IS_DEBUG)
	{
		auto dataCopy = m_cbData;
		const auto pos = XMLoadFloat3(&m_cbData.pos);
		XMStoreFloat3(&dataCopy.pos, XMVector3Transform(pos, view));

		auto shadowDataCopy = m_shadowData;
		const auto cameraPos = m_pCamera->GetPos();
		const auto ViewProj = XMMatrixTranspose(
			XMMatrixTranslation(-cameraPos.x, -cameraPos.y, -cameraPos.z)
		);
		shadowDataCopy.ViewProj = ViewProj;

		Drawable::m_lightShadowBindable->Update(gfx, &shadowDataCopy);
		Drawable::m_lightBindable->Update(gfx, &dataCopy);
	}

	void PointLight::LinkTechniques(RenderGraph& rg)
	{
		m_indicator.LinkTechniques(rg);
	}

	std::shared_ptr<Camera> PointLight::ShareCamera() const noexcept(!IS_DEBUG)
	{
		return m_pCamera;
	}
}