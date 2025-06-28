#include "Camera.h"

namespace Renderer
{
	Camera::Camera(D3D12RHI& gfx, std::string name, Transform transform) noexcept(!IS_DEBUG)
		:
		m_name(std::move(name)),
		m_homeTransform(transform),
		m_transform(transform),
		m_homeProjection(Projection{ 1.0f, 9.0f / 16.0f , 0.5f, 400.0f})
	{
		m_state.rotate = m_state.move = true;

		m_projection = m_homeProjection;
	}

	void Camera::Update(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		SetViewMatrix();
		SetProjectionMatrix();

		m_cameraData.viewMat = GetViewMatrix();
		m_cameraData.projectionMat = GetProjectionMatrix();
		m_cameraData.inverseViewMat = m_cameraData.viewMat.Invert();
		m_cameraData.inverseProjectionMat = m_cameraData.projectionMat.Invert();
		m_cameraData.inverseViewProjectionMat = (m_cameraData.viewMat * m_cameraData.projectionMat).Invert();
		m_cameraData.position = GetTransform().position;
	}

	Matrix Camera::GetViewMatrix() const noexcept(!IS_DEBUG)
	{		
		return m_viewMatrix;
	}

	Matrix Camera::GetProjectionMatrix() const noexcept(!IS_DEBUG)
	{
		return m_projectionMatrix;
	}

	void Camera::SetViewMatrix() noexcept(!IS_DEBUG)
	{
		const XMVECTOR forwardBaseVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		// apply the camera rotations to a base vector
		const auto lookVector = XMVector3Transform(forwardBaseVector,
			XMMatrixRotationRollPitchYaw(m_transform.rotation.x, m_transform.rotation.y, m_transform.rotation.z)
		);
		// generate camera transform (applied to all objects to arrange them relative
		// to camera position/orientation in world) from cam position and direction
		// camera "top" always faces towards +Y (cannot do a barrel roll)
		const auto camPosition = XMLoadFloat3(&m_transform.position);
		const auto camTarget = camPosition + lookVector;
		m_viewMatrix = XMMatrixLookAtLH(camPosition, camTarget, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	}

	void Camera::SetProjectionMatrix() noexcept(!IS_DEBUG)
	{
		m_projectionMatrix = XMMatrixPerspectiveFovLH(m_fov, m_projection.width / m_projection.height, m_projection.nearZ, m_projection.farZ);
	}

	void Camera::SpawnControlWidgets(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		const auto dcheck = [](bool d, bool& carry) { carry = carry || d; };
		ImGui::Text("Position");
		dcheck(ImGui::SliderFloat("X", &m_transform.position.x, -80.0f, 80.0f, "%.1f"), m_state.move);
		dcheck(ImGui::SliderFloat("Y", &m_transform.position.y, -80.0f, 80.0f, "%.1f"), m_state.move);
		dcheck(ImGui::SliderFloat("Z", &m_transform.position.z, -80.0f, 80.0f, "%.1f"), m_state.move);
		ImGui::Text("Orientation");
		dcheck(ImGui::SliderAngle("Pitch", &m_transform.rotation.x, 0.995f * -90.0f, 0.995f * 90.0f), m_state.rotate);
		dcheck(ImGui::SliderAngle("Yaw", &m_transform.rotation.x, -180.0f, 180.0f), m_state.rotate);
		ImGui::Text("Projection");
		dcheck(ImGui::SliderFloat("Width", &m_projection.width, 0.01f, 4.0f, "%.2f"), m_state.project);
		dcheck(ImGui::SliderFloat("Height", &m_projection.height, 0.01f, 4.0f, "%.2f"), m_state.project);
		dcheck(ImGui::SliderFloat("Near Z", &m_projection.nearZ, 0.01f, m_projection.farZ - 0.01f, "%.2f"), m_state.project);
		dcheck(ImGui::SliderFloat("Far Z", &m_projection.farZ, m_projection.nearZ + 0.01f, 400.0f, "%.2f"), m_state.project);
		ImGui::Checkbox("Camera Indicator", &m_state.enableCameraIndicator);
		ImGui::Checkbox("Camera Projection", &m_state.enableCameraProjection);
		if (ImGui::Button("Reset"))
		{
			Reset(gfx);
		}

		SetViewMatrix();
		SetProjectionMatrix();
	}

	void Camera::Reset(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		m_transform.position = m_homeTransform.position;
		m_transform.rotation = m_homeTransform.rotation;
		m_projection = m_homeProjection;

		m_state.rotate = m_state.move = m_state.project = true;
	}

	void Camera::Rotate(Vector2 rotation) noexcept(!IS_DEBUG)
	{
		m_transform.rotation.x = std::clamp(m_transform.rotation.x + rotation.y * m_rotationSpeed, 0.995f * -PI / 2.0f, 0.995f * PI / 2.0f);
		m_transform.rotation.y = wrap_angle(m_transform.rotation.y + rotation.x * m_rotationSpeed);

		m_state.rotate = true;
	}

	void Camera::Translate(Vector3 translation) noexcept(!IS_DEBUG)
	{
		XMStoreFloat3(&translation, XMVector3Transform(
			XMLoadFloat3(&translation),
			XMMatrixRotationRollPitchYaw(m_transform.rotation.x, m_transform.rotation.y, m_transform.rotation.z) *
			XMMatrixScaling(m_travelSpeed, m_travelSpeed, m_travelSpeed)
		));

		m_transform.position = {
			m_transform.position.x + translation.x,
			m_transform.position.y + translation.y,
			m_transform.position.z + translation.z
		};

		m_state.move = true;
	}
}