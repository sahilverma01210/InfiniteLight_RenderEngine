#include "Camera.h"
#include "imgui/imgui.h"

namespace Renderer
{
	XMMATRIX Camera::GetMatrix() const noexcept
	{
		const auto pos = XMVector3Transform(
			XMVectorSet(0.0f, 0.0f, -r, 0.0f),
			XMMatrixRotationRollPitchYaw(phi, -theta, 0.0f)
		);
		return XMMatrixLookAtLH(
			pos, XMVectorZero(),
			XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
		) * XMMatrixRotationRollPitchYaw(
			pitch, -yaw, roll
		);
	}

	bool Camera::SpawnControlWindow(D3D12RHI& gfx) noexcept
	{
		if (ImGui::Begin("Camera", &m_imGUIwndOpen))
		{
			ImGui::Text("Position");
			ImGui::SliderFloat("R", &r, 0.2f, 80.0f, "%.1f");
			ImGui::SliderAngle("Theta", &theta, -180.0f, 180.0f);
			ImGui::SliderAngle("Phi", &phi, -89.0f, 89.0f);
			ImGui::Text("Orientation");
			ImGui::SliderAngle("Roll", &roll, -180.0f, 180.0f);
			ImGui::SliderAngle("Pitch", &pitch, -180.0f, 180.0f);
			ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f);
			if (ImGui::Button("Reset"))
			{
				Reset(gfx);
			}
		}
		ImGui::End();

		return m_imGUIwndOpen;
	}

	void Camera::Update(D3D12RHI& gfx) noexcept
	{
		gfx.SetCamera(GetMatrix());
		gfx.SetProjection(XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 40.0f));
	}

	void Camera::Reset(D3D12RHI& gfx) noexcept
	{
		r = 20.0f;
		theta = 0.0f;
		phi = 0.0f;
		pitch = 0.0f;
		yaw = 0.0f;
		roll = 0.0f;

		gfx.SetCamera(GetMatrix());
		gfx.SetProjection(XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 40.0f));
	}
}