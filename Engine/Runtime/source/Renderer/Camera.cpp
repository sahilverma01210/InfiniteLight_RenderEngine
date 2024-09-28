#include "Camera.h"

namespace Renderer
{
	Camera::Camera(D3D12RHI& gfx) noexcept
	{
		Reset(gfx);
	}

	XMMATRIX Camera::GetMatrix() const noexcept
	{
		const XMVECTOR forwardBaseVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		// apply the camera rotations to a base vector
		const auto lookVector = XMVector3Transform(forwardBaseVector,
			XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f)
		);
		// generate camera transform (applied to all objects to arrange them relative
		// to camera position/orientation in world) from cam position and direction
		// camera "top" always faces towards +Y (cannot do a barrel roll)
		const auto camPosition = XMLoadFloat3(&pos);
		const auto camTarget = camPosition + lookVector;
		return XMMatrixLookAtLH(camPosition, camTarget, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	}

	bool Camera::SpawnControlWindow(D3D12RHI& gfx) noexcept
	{
		if (ImGui::Begin("Camera"))
		{
			ImGui::Text("Position");
			ImGui::SliderFloat("X", &pos.x, -80.0f, 80.0f, "%.1f");
			ImGui::SliderFloat("Y", &pos.y, -80.0f, 80.0f, "%.1f");
			ImGui::SliderFloat("Z", &pos.z, -80.0f, 80.0f, "%.1f");
			ImGui::Text("Orientation");
			ImGui::SliderAngle("Pitch", &pitch, 0.995f * -90.0f, 0.995f * 90.0f);
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
		gfx.SetProjection(XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 400.0f));
	}

	void Camera::Reset(D3D12RHI& gfx) noexcept
	{
		pos = { -13.5f,6.0f,3.5f };
		pitch = 0.0f;
		yaw = PI / 2.0f;

		gfx.SetCamera(GetMatrix());
		gfx.SetProjection(XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 40.0f));
	}

	void Camera::Rotate(float dx, float dy) noexcept
	{
		yaw = wrap_angle(yaw + dx * rotationSpeed);
		pitch = std::clamp(pitch + dy * rotationSpeed, 0.995f * -PI / 2.0f, 0.995f * PI / 2.0f);
	}

	void Camera::Translate(XMFLOAT3 translation) noexcept
	{
		XMStoreFloat3(&translation, XMVector3Transform(
			XMLoadFloat3(&translation),
			XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f) *
			XMMatrixScaling(travelSpeed, travelSpeed, travelSpeed)
		));
		pos = {
			pos.x + translation.x,
			pos.y + translation.y,
			pos.z + translation.z
		};
	}
}