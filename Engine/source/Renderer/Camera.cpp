#include "Camera.h"

namespace Renderer
{
	Camera::Camera(D3D12RHI& gfx, std::string name, XMFLOAT3 homePos, float homePitch, float homeYaw, bool tethered) noexcept(!IS_DEBUG)
		:
		name(std::move(name)),
		homePos(homePos),
		homePitch(homePitch),
		homeYaw(homeYaw),
		proj(gfx, 1.0f, 9.0f / 16.0f, 0.5f, 400.0f),
		indicator(gfx),
		tethered(tethered)
	{
		if (tethered)
		{
			pos = homePos;
			indicator.SetPos(pos);
			proj.SetPos(pos);
		}
		gfx.ResetCommandList();
		Reset(gfx);
		gfx.ExecuteCommandList();
	}

	XMMATRIX Camera::GetMatrix() const noexcept(!IS_DEBUG)
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

	XMMATRIX Camera::GetProjection() const noexcept(!IS_DEBUG)
	{
		return proj.GetMatrix();
	}

	void Camera::SpawnControlWidgets(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		bool rotDirty = false;
		bool posDirty = false;
		const auto dcheck = [](bool d, bool& carry) { carry = carry || d; };
		if (!tethered)
		{
			ImGui::Text("Position");
			dcheck(ImGui::SliderFloat("X", &pos.x, -80.0f, 80.0f, "%.1f"), posDirty);
			dcheck(ImGui::SliderFloat("Y", &pos.y, -80.0f, 80.0f, "%.1f"), posDirty);
			dcheck(ImGui::SliderFloat("Z", &pos.z, -80.0f, 80.0f, "%.1f"), posDirty);
		}
		ImGui::Text("Orientation");
		dcheck(ImGui::SliderAngle("Pitch", &pitch, 0.995f * -90.0f, 0.995f * 90.0f), rotDirty);
		dcheck(ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f), rotDirty);
		proj.RenderWidgets(gfx);
		ImGui::Checkbox("Camera Indicator", &enableCameraIndicator);
		ImGui::Checkbox("Frustum Indicator", &enableFrustumIndicator);
		if (ImGui::Button("Reset"))
		{
			Reset(gfx);
		}
		if (rotDirty)
		{
			const XMFLOAT3 angles = { pitch,yaw,0.0f };
			indicator.SetRotation(angles);
			proj.SetRotation(angles);
		}
		if (posDirty)
		{
			indicator.SetPos(pos);
			proj.SetPos(pos);
		}
	}

	void Camera::Update(D3D12RHI& gfx) const
	{
		gfx.SetCamera(GetMatrix());
		gfx.SetProjection(proj.GetMatrix());
	}

	void Camera::Reset(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		if (!tethered)
		{
			pos = homePos;
			indicator.SetPos(pos);
			proj.SetPos(pos);
		}

		pitch = homePitch;
		yaw = homeYaw;

		const XMFLOAT3 angles = { pitch,yaw,0.0f };
		indicator.SetRotation(angles);
		proj.SetRotation(angles);
		proj.Reset(gfx);

		gfx.SetCamera(GetMatrix());
		gfx.SetProjection(XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 40.0f));
	}

	void Camera::Rotate(float dx, float dy) noexcept(!IS_DEBUG)
	{
		yaw = wrap_angle(yaw + dx * rotationSpeed);
		pitch = std::clamp(pitch + dy * rotationSpeed, 0.995f * -PI / 2.0f, 0.995f * PI / 2.0f);
		const XMFLOAT3 angles = { pitch,yaw,0.0f };
		indicator.SetRotation(angles);
		proj.SetRotation(angles);
	}

	void Camera::Translate(XMFLOAT3 translation) noexcept(!IS_DEBUG)
	{
		if (!tethered)
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
			indicator.SetPos(pos);
			proj.SetPos(pos);
		}
	}

	XMFLOAT3 Camera::GetPos() const noexcept(!IS_DEBUG)
	{
		return pos;
	}

	void Camera::SetPos(const XMFLOAT3& pos) noexcept(!IS_DEBUG)
	{
		this->pos = pos;
		indicator.SetPos(pos);
		proj.SetPos(pos);
	}

	const std::string& Camera::GetName() const noexcept(!IS_DEBUG)
	{
		return name;
	}

	void Camera::LinkTechniques(RenderGraph& rg)
	{
		indicator.LinkTechniques(rg);
		proj.LinkTechniques(rg);
	}

	void Camera::Submit(size_t channels) const
	{
		if (enableCameraIndicator)
		{
			indicator.Submit(channels);
		}
		if (enableFrustumIndicator)
		{
			proj.Submit(channels);
		}
	}
}