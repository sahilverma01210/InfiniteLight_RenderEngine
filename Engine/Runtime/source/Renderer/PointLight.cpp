#include "PointLight.h"

namespace Renderer
{
	PointLight::PointLight(D3D12RHI& gfx, float radius)
		:
		mesh(gfx, radius)
	{
		Reset();

		std::unique_ptr<Bindable> cbuf = std::make_unique<ConstantBuffer>(gfx, 1, sizeof(cbData), &cbData);
		pLightBindable = cbuf.get();
		SolidSphere::staticBinds.push_back(std::move(cbuf));
	}

	bool PointLight::SpawnControlWindow() noexcept
	{
		if (ImGui::Begin("Light", &m_imGUIwndOpen))
		{
			ImGui::Text("Position");
			ImGui::SliderFloat("X", &cbData.pos.x, -60.0f, 60.0f, "%.1f");
			ImGui::SliderFloat("Y", &cbData.pos.y, -60.0f, 60.0f, "%.1f");
			ImGui::SliderFloat("Z", &cbData.pos.z, -60.0f, 60.0f, "%.1f");

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
		cbData = { 
			{ 0.0f,0.0f,0.0f },
			{ 0.05f,0.05f,0.05f },
			{ 1.0f,1.0f,1.0f },
			1.0f,
			1.0f,
			0.045f,
			0.0075f
		};
	}

	void PointLight::Draw(D3D12RHI& gfx, Camera& camera) const noexcept
	{
		mesh.SetPos(cbData.pos);
		gfx.SetTransform(mesh.GetTransformXM());
		gfx.SetCamera(camera.GetMatrix());
		gfx.SetProjection(XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
		mesh.Draw(gfx);
	}

	void PointLight::Bind(D3D12RHI& gfx, FXMMATRIX view) const noexcept
	{
		auto dataCopy = cbData;
		const auto pos = XMLoadFloat3(&cbData.pos);
		XMStoreFloat3(&dataCopy.pos, XMVector3Transform(pos, view));
		pLightBindable->Update(gfx, &dataCopy);
		//cbuf.Update(gfx, &pos);
		//cbuf.Bind(gfx);
	}
}