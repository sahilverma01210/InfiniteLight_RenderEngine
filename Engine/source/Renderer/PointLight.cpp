#include "PointLight.h"

namespace Renderer
{
	PointLight::PointLight(D3D12RHI& gfx, XMFLOAT3 pos, float radius)
		:
		mesh(gfx, radius)
	{
		home = {
			pos,
			{ 0.05f,0.05f,0.05f },
			{ 1.0f,1.0f,1.0f },
			1.0f,
			1.0f,
			0.025f,
			0.0030f,
		};

		Reset();
		pCamera = std::make_shared<Camera>(gfx, "Light", cbData.pos, 0.0f, PI / 2.0f, true);
		Drawable::lightShadowBindable = std::move(std::make_unique<ConstantBuffer>(gfx, 1, sizeof(shadowData), &shadowData));
		Drawable::lightBindable = std::move(std::make_unique<ConstantBuffer>(gfx, 2, sizeof(cbData), &cbData));
	}

	bool PointLight::SpawnControlWindow() noexcept
	{
		if (ImGui::Begin("Light", &m_imGUIwndOpen))
		{
			bool dirtyPos = false;
			const auto d = [&dirtyPos](bool dirty) {dirtyPos = dirtyPos || dirty; };

			ImGui::Text("Position");
			d(ImGui::SliderFloat("X", &cbData.pos.x, -60.0f, 60.0f, "%.1f"));
			d(ImGui::SliderFloat("Y", &cbData.pos.y, -60.0f, 60.0f, "%.1f"));
			d(ImGui::SliderFloat("Z", &cbData.pos.z, -60.0f, 60.0f, "%.1f"));
			if (dirtyPos)
			{
				pCamera->SetPos(cbData.pos);
			}
			ImGui::Text("Intensity/Color");
			ImGui::SliderFloat("Intensity", &cbData.diffuseIntensity, 0.01f, 2.0f, "%.2f");
			ImGui::ColorEdit3("Diffuse Color", &cbData.diffuseColor.x);
			ImGui::ColorEdit3("Ambient", &cbData.ambient.x);

			ImGui::Text("Falloff");
			ImGui::SliderFloat("Constant", &cbData.attConst, 0.05f, 10.0f, "%.2f");
			ImGui::SliderFloat("Linear", &cbData.attLin, 0.0001f, 4.0f, "%.4f");
			ImGui::SliderFloat("Quadratic", &cbData.attQuad, 0.0000001f, 10.0f, "%.7f");

			if (ImGui::Button("Reset"))
			{
				Reset();
			}
		}
		ImGui::End();

		return m_imGUIwndOpen;
	}

	void PointLight::Reset() noexcept
	{
		cbData = home;
	}

	void PointLight::Submit(size_t channels) const noexcept
	{
		mesh.SetPos(cbData.pos);
		mesh.Submit(channels);
	}

	void PointLight::Update(D3D12RHI& gfx, FXMMATRIX view) const noexcept
	{
		auto dataCopy = cbData;
		const auto pos = XMLoadFloat3(&cbData.pos);
		XMStoreFloat3(&dataCopy.pos, XMVector3Transform(pos, view));

		auto shadowDataCopy = shadowData;
		const auto cameraPos = pCamera->GetPos();
		const auto ViewProj = XMMatrixTranspose(
			XMMatrixTranslation(-cameraPos.x, -cameraPos.y, -cameraPos.z)
		);
		shadowDataCopy.ViewProj = ViewProj;

		Drawable::lightShadowBindable->Update(gfx, &shadowDataCopy);
		Drawable::lightBindable->Update(gfx, &dataCopy);
	}

	void PointLight::LinkTechniques(RenderGraph& rg)
	{
		mesh.LinkTechniques(rg);
	}

	std::shared_ptr<Camera> PointLight::ShareCamera() const noexcept
	{
		return pCamera;
	}
}