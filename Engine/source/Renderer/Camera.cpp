#include "Camera.h"

namespace Renderer
{
	XMMATRIX ILMesh::m_cameraMatrix;
	XMMATRIX ILMesh::m_projectionMatrix;

	Camera::Camera(D3D12RHI& gfx, std::string name, Transform transform, bool tethered) noexcept(!IS_DEBUG)
		:
		m_name(std::move(name)),
		m_homeTransform(transform),
		m_tethered(tethered),
		m_projection(gfx, CameraProjection::Projection{ 1.0f, 9.0f / 16.0f, 0.5f, 400.0f }),
		m_indicator(gfx)
	{
		if (tethered) 
		{
			m_transform.position = m_homeTransform.position;
			m_indicator.SetPos(m_transform.position);
			m_projection.SetPos(m_transform.position);
		}

		gfx.ResetCommandList();
		Reset(gfx);
		gfx.ExecuteCommandList();
	}

	void Camera::Update(bool has360View, UINT direction) const noexcept(!IS_DEBUG)
	{
		ILMesh::m_cameraMatrix = has360View ? Get360CameraMatrix(direction) : GetCameraMatrix();
		ILMesh::m_projectionMatrix = has360View ? Get360ProjectionMatrix() : GetProjectionMatrix();
	}

	XMMATRIX Camera::GetCameraMatrix() const noexcept(!IS_DEBUG)
	{
		const XMVECTOR forwardBaseVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		// apply the camera rotations to a base vector
		const auto lookVector = XMVector3Transform(forwardBaseVector,
			XMMatrixRotationRollPitchYaw(m_transform.pitch, m_transform.yaw, 0.0f)
		);
		// generate camera transform (applied to all objects to arrange them relative
		// to camera position/orientation in world) from cam position and direction
		// camera "top" always faces towards +Y (cannot do a barrel roll)
		const auto camPosition = XMLoadFloat3(&m_transform.position);
		const auto camTarget = camPosition + lookVector;
		return XMMatrixLookAtLH(camPosition, camTarget, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	}

	XMMATRIX Camera::GetProjectionMatrix() const noexcept(!IS_DEBUG)
	{
		return m_projection.GetProjectionMatrix();
	}

	XMMATRIX Camera::Get360CameraMatrix(UINT directionIndex) const noexcept(!IS_DEBUG)
	{
		const auto cameraPosition = XMLoadFloat3(&m_transform.position);

		switch (directionIndex)
		{
		case 0: // +x			
			return XMMatrixLookAtLH(cameraPosition, cameraPosition + XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			break;
		case 1: // -x			
			return XMMatrixLookAtLH(cameraPosition, cameraPosition + XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			break;
		case 2: // +y			
			return XMMatrixLookAtLH(cameraPosition, cameraPosition + XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
			break;
		case 3: // -y			
			return XMMatrixLookAtLH(cameraPosition, cameraPosition + XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
			break;
		case 4: // +z			
			return XMMatrixLookAtLH(cameraPosition, cameraPosition + XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			break;
		case 5: // -z			
			return XMMatrixLookAtLH(cameraPosition, cameraPosition + XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			break;
		default:
			return XMMATRIX();
			break;
		}
	}

	XMMATRIX Camera::Get360ProjectionMatrix() const noexcept(!IS_DEBUG)
	{
		return XMMatrixPerspectiveFovLH(PI / 2.0f, 1.0f, 0.5f, 100.0f);
	}

	void Camera::SpawnControlWidgets(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		bool rotDirty = false;
		bool posDirty = false;
		const auto dcheck = [](bool d, bool& carry) { carry = carry || d; };
		if (!m_tethered)
		{
			ImGui::Text("Position");
			dcheck(ImGui::SliderFloat("X", &m_transform.position.x, -80.0f, 80.0f, "%.1f"), posDirty);
			dcheck(ImGui::SliderFloat("Y", &m_transform.position.y, -80.0f, 80.0f, "%.1f"), posDirty);
			dcheck(ImGui::SliderFloat("Z", &m_transform.position.z, -80.0f, 80.0f, "%.1f"), posDirty);
		}
		ImGui::Text("Orientation");
		dcheck(ImGui::SliderAngle("Pitch", &m_transform.pitch, 0.995f * -90.0f, 0.995f * 90.0f), rotDirty);
		dcheck(ImGui::SliderAngle("Yaw", &m_transform.yaw, -180.0f, 180.0f), rotDirty);
		m_projection.RenderWidgets(gfx);
		ImGui::Checkbox("Camera Indicator", &m_enableCameraIndicator);
		ImGui::Checkbox("Frustum Indicator", &m_enableFrustumIndicator);
		if (ImGui::Button("Reset"))
		{
			Reset(gfx);
		}
		if (rotDirty)
		{
			const XMFLOAT3 angles = { m_transform.pitch,m_transform.yaw,0.0f };
			m_indicator.SetRotation(angles);
			m_projection.SetRotation(angles);
		}
		if (posDirty)
		{
			m_indicator.SetPos(m_transform.position);
			m_projection.SetPos(m_transform.position);
		}
	}

	void Camera::Reset(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		if (!m_tethered)
		{
			m_transform.position = m_homeTransform.position;
			m_indicator.SetPos(m_transform.position);
			m_projection.SetPos(m_transform.position);
		}

		m_transform.pitch = m_homeTransform.pitch;
		m_transform.yaw = m_homeTransform.yaw;

		const XMFLOAT3 angles = { m_transform.pitch,m_transform.yaw,0.0f };
		m_indicator.SetRotation(angles);
		m_projection.SetRotation(angles);
		m_projection.Reset(gfx);
	}

	void Camera::Rotate(float dx, float dy) noexcept(!IS_DEBUG)
	{
		m_transform.yaw = wrap_angle(m_transform.yaw + dx * m_rotationSpeed);
		m_transform.pitch = std::clamp(m_transform.pitch + dy * m_rotationSpeed, 0.995f * -PI / 2.0f, 0.995f * PI / 2.0f);
		const XMFLOAT3 angles = { m_transform.pitch,m_transform.yaw,0.0f };
		m_indicator.SetRotation(angles);
		m_projection.SetRotation(angles);
	}

	void Camera::Translate(XMFLOAT3 translation) noexcept(!IS_DEBUG)
	{
		if (!m_tethered)
		{
			XMStoreFloat3(&translation, XMVector3Transform(
				XMLoadFloat3(&translation),
				XMMatrixRotationRollPitchYaw(m_transform.pitch, m_transform.yaw, 0.0f) *
				XMMatrixScaling(m_travelSpeed, m_travelSpeed, m_travelSpeed)
			));
			m_transform.position = {
				m_transform.position.x + translation.x,
				m_transform.position.y + translation.y,
				m_transform.position.z + translation.z
			};
			m_indicator.SetPos(m_transform.position);
			m_projection.SetPos(m_transform.position);
		}
	}

	XMFLOAT3 Camera::GetPos() const noexcept(!IS_DEBUG)
	{
		return m_transform.position;
	}

	void Camera::SetPos(const XMFLOAT3& pos) noexcept(!IS_DEBUG)
	{
		this->m_transform.position = pos;
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

	void Camera::Submit(size_t channel) const
	{
		if (m_enableCameraIndicator)
		{
			m_indicator.Submit(channel);
		}
		if (m_enableFrustumIndicator)
		{
			m_projection.Submit(channel);
		}
	}
}