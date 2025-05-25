#include "Camera.h"

namespace Renderer
{
	UINT ILMesh::m_meshCount;

	Camera::Camera(D3D12RHI& gfx, std::string name, Transform transform, bool tethered) noexcept(!IS_DEBUG)
		:
		m_name(std::move(name)),
		m_homeTransform(transform),
		m_transform(transform),
		m_tethered(tethered),
		m_homeProjection(CameraProjection::Projection{ 1.0f, 9.0f / 16.0f , 0.5f, 400.0f}),
		m_cameraProjection(gfx, m_homeProjection),
		m_cameraIndicator(gfx)
	{
		m_cameraIndicator.SetPos(m_transform.position);
		m_cameraProjection.SetPos(m_transform.position);
		m_cameraIndicator.SetRotation(m_transform.rotation);
		m_cameraProjection.SetRotation(m_transform.rotation);

		m_projection = m_homeProjection;
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
		struct CameraProbe : public TechniqueProbe
		{
			virtual void OnSetTechnique()
			{
				if (m_pTech->name == "line_wire")
				{
					m_pTech->active = highlighted;
				}
			}
			bool highlighted = false;
		} probe;
		
		bool rotDirty = false;
		bool posDirty = false;
		bool projDirty = false;
		const auto dcheck = [](bool d, bool& carry) { carry = carry || d; };
		if (!m_tethered)
		{
			ImGui::Text("Position");
			dcheck(ImGui::SliderFloat("X", &m_transform.position.x, -80.0f, 80.0f, "%.1f"), posDirty);
			dcheck(ImGui::SliderFloat("Y", &m_transform.position.y, -80.0f, 80.0f, "%.1f"), posDirty);
			dcheck(ImGui::SliderFloat("Z", &m_transform.position.z, -80.0f, 80.0f, "%.1f"), posDirty);
		}
		ImGui::Text("Orientation");
		dcheck(ImGui::SliderAngle("Pitch", &m_transform.rotation.x, 0.995f * -90.0f, 0.995f * 90.0f), rotDirty);
		dcheck(ImGui::SliderAngle("Yaw", &m_transform.rotation.x, -180.0f, 180.0f), rotDirty);
		ImGui::Text("Projection");
		dcheck(ImGui::SliderFloat("Width", &m_projection.width, 0.01f, 4.0f, "%.2f"), projDirty);
		dcheck(ImGui::SliderFloat("Height", &m_projection.height, 0.01f, 4.0f, "%.2f"), projDirty);
		dcheck(ImGui::SliderFloat("Near Z", &m_projection.nearZ, 0.01f, m_projection.farZ - 0.01f, "%.2f"), projDirty);
		dcheck(ImGui::SliderFloat("Far Z", &m_projection.farZ, m_projection.nearZ + 0.01f, 400.0f, "%.2f"), projDirty);
		ImGui::Checkbox("Camera Indicator", &m_enableCameraIndicator);
		ImGui::Checkbox("Camera Projection", &m_enableCameraProjection);
		if (ImGui::Button("Reset"))
		{
			Reset(gfx);
		}
		if (rotDirty)
		{
			m_cameraIndicator.SetRotation(m_transform.rotation);
			m_cameraProjection.SetRotation(m_transform.rotation);
		}
		if (posDirty)
		{
			m_cameraIndicator.SetPos(m_transform.position);
			m_cameraProjection.SetPos(m_transform.position);
		}
		if (projDirty)
		{
			m_cameraProjection.SetVertices(gfx, m_projection);
		}
		
		if (m_enableCameraIndicator)
		{
			probe.highlighted = true;
			m_cameraIndicator.Accept(probe);
		}
		
		if (m_enableCameraProjection)
		{
			probe.highlighted = true;
			m_cameraProjection.Accept(probe);
		}

		SetViewMatrix();
		SetProjectionMatrix();
	}

	void Camera::Reset(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		if (!m_tethered)
		{
			m_transform.position = m_homeTransform.position;
			m_cameraIndicator.SetPos(m_transform.position);
			m_cameraProjection.SetPos(m_transform.position);
		}

		m_transform.rotation = m_homeTransform.rotation;

		m_cameraIndicator.SetRotation(m_transform.rotation);
		m_cameraProjection.SetRotation(m_transform.rotation);
		m_projection = m_homeProjection;

		m_cameraProjection.SetVertices(gfx, m_projection);
	}

	void Camera::Rotate(float dx, float dy) noexcept(!IS_DEBUG)
	{
		m_transform.rotation.x = std::clamp(m_transform.rotation.x + dy * m_rotationSpeed, 0.995f * -PI / 2.0f, 0.995f * PI / 2.0f);
		m_transform.rotation.y = wrap_angle(m_transform.rotation.y + dx * m_rotationSpeed);

		m_cameraIndicator.SetRotation(m_transform.rotation);
		m_cameraProjection.SetRotation(m_transform.rotation);
	}

	void Camera::Translate(Vector3 translation, float dt) noexcept(!IS_DEBUG)
	{
		if (!m_tethered)
		{
			translation *= dt;

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
			m_cameraIndicator.SetPos(m_transform.position);
			m_cameraProjection.SetPos(m_transform.position);
		}
	}

	Vector3 Camera::GetPos() const noexcept(!IS_DEBUG)
	{
		return m_transform.position;
	}

	void Camera::SetPos(const Vector3& pos) noexcept(!IS_DEBUG)
	{
		this->m_transform.position = pos;
		m_cameraIndicator.SetPos(pos);
		m_cameraProjection.SetPos(pos);
	}

	const std::string& Camera::GetName() const noexcept(!IS_DEBUG)
	{
		return m_name;
	}

	void Camera::Submit(RenderGraph& renderGraph) const
	{
		if (m_enableCameraIndicator)
		{
			m_cameraIndicator.Submit(renderGraph);
		}
		if (m_enableCameraProjection)
		{
			m_cameraProjection.Submit(renderGraph);
		}
	}
}