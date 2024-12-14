#include "Camera.h"

namespace Renderer
{
	Camera::Camera(D3D12RHI& gfx, std::string name, XMFLOAT3 homePos, float homePitch, float homeYaw, bool tethered) noexcept(!IS_DEBUG)
		:
		m_name(std::move(name)),
		m_homePos(homePos),
		m_homePitch(homePitch),
		m_homeYaw(homeYaw),
		m_projection(gfx, 1.0f, 9.0f / 16.0f, 0.5f, 400.0f),
		m_indicator(gfx),
		m_tethered(tethered)
	{
		if (tethered)
		{
			m_pos = homePos;
			m_indicator.SetPos(m_pos);
			m_projection.SetPos(m_pos);
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
			XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f)
		);
		// generate camera transform (applied to all objects to arrange them relative
		// to camera position/orientation in world) from cam position and direction
		// camera "top" always faces towards +Y (cannot do a barrel roll)
		const auto camPosition = XMLoadFloat3(&m_pos);
		const auto camTarget = camPosition + lookVector;
		return XMMatrixLookAtLH(camPosition, camTarget, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	}

	XMMATRIX Camera::GetProjection() const noexcept(!IS_DEBUG)
	{
		return m_projection.GetMatrix();
	}

	void Camera::SpawnControlWidgets(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		bool rotDirty = false;
		bool posDirty = false;
		const auto dcheck = [](bool d, bool& carry) { carry = carry || d; };
		if (!m_tethered)
		{
			ImGui::Text("Position");
			dcheck(ImGui::SliderFloat("X", &m_pos.x, -80.0f, 80.0f, "%.1f"), posDirty);
			dcheck(ImGui::SliderFloat("Y", &m_pos.y, -80.0f, 80.0f, "%.1f"), posDirty);
			dcheck(ImGui::SliderFloat("Z", &m_pos.z, -80.0f, 80.0f, "%.1f"), posDirty);
		}
		ImGui::Text("Orientation");
		dcheck(ImGui::SliderAngle("Pitch", &m_pitch, 0.995f * -90.0f, 0.995f * 90.0f), rotDirty);
		dcheck(ImGui::SliderAngle("Yaw", &m_yaw, -180.0f, 180.0f), rotDirty);
		m_projection.RenderWidgets(gfx);
		ImGui::Checkbox("Camera Indicator", &m_enableCameraIndicator);
		ImGui::Checkbox("Frustum Indicator", &m_enableFrustumIndicator);
		if (ImGui::Button("Reset"))
		{
			Reset(gfx);
		}
		if (rotDirty)
		{
			const XMFLOAT3 angles = { m_pitch,m_yaw,0.0f };
			m_indicator.SetRotation(angles);
			m_projection.SetRotation(angles);
		}
		if (posDirty)
		{
			m_indicator.SetPos(m_pos);
			m_projection.SetPos(m_pos);
		}
	}

	void Camera::Update(D3D12RHI& gfx) const
	{
		gfx.SetCamera(GetMatrix());
		gfx.SetProjection(m_projection.GetMatrix());
	}

	void Camera::Reset(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		if (!m_tethered)
		{
			m_pos = m_homePos;
			m_indicator.SetPos(m_pos);
			m_projection.SetPos(m_pos);
		}

		m_pitch = m_homePitch;
		m_yaw = m_homeYaw;

		const XMFLOAT3 angles = { m_pitch,m_yaw,0.0f };
		m_indicator.SetRotation(angles);
		m_projection.SetRotation(angles);
		m_projection.Reset(gfx);

		gfx.SetCamera(GetMatrix());
		gfx.SetProjection(XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 40.0f));
	}

	void Camera::Rotate(float dx, float dy) noexcept(!IS_DEBUG)
	{
		m_yaw = wrap_angle(m_yaw + dx * m_rotationSpeed);
		m_pitch = std::clamp(m_pitch + dy * m_rotationSpeed, 0.995f * -PI / 2.0f, 0.995f * PI / 2.0f);
		const XMFLOAT3 angles = { m_pitch,m_yaw,0.0f };
		m_indicator.SetRotation(angles);
		m_projection.SetRotation(angles);
	}

	void Camera::Translate(XMFLOAT3 translation) noexcept(!IS_DEBUG)
	{
		if (!m_tethered)
		{
			XMStoreFloat3(&translation, XMVector3Transform(
				XMLoadFloat3(&translation),
				XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f) *
				XMMatrixScaling(m_travelSpeed, m_travelSpeed, m_travelSpeed)
			));
			m_pos = {
				m_pos.x + translation.x,
				m_pos.y + translation.y,
				m_pos.z + translation.z
			};
			m_indicator.SetPos(m_pos);
			m_projection.SetPos(m_pos);
		}
	}

	XMFLOAT3 Camera::GetPos() const noexcept(!IS_DEBUG)
	{
		return m_pos;
	}

	void Camera::SetPos(const XMFLOAT3& pos) noexcept(!IS_DEBUG)
	{
		this->m_pos = pos;
		m_indicator.SetPos(pos);
		m_projection.SetPos(pos);
	}

	const std::string& Camera::GetName() const noexcept(!IS_DEBUG)
	{
		return m_name;
	}

	void Camera::LinkTechniques(RenderGraph& rg)
	{
		m_indicator.LinkTechniques(rg);
		m_projection.LinkTechniques(rg);
	}

	void Camera::Submit(size_t channels) const
	{
		if (m_enableCameraIndicator)
		{
			m_indicator.Submit(channels);
		}
		if (m_enableFrustumIndicator)
		{
			m_projection.Submit(channels);
		}
	}
}