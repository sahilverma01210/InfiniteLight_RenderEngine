#pragma once
#include "../Common/ImGUI_Includes.h"

#include "RenderMath.h"
#include "CameraIndicator.h"
#include "CameraProjection.h"

namespace Renderer
{
	class Camera
	{
	public:
		struct Transform
		{
			Vector3 position = { 0.0f,0.0f,0.0f }; // X, Y, Z
			Vector3 rotation = { 0.0f,0.0f,0.0f }; // Pitch, Yaw, Roll
			Vector3 velocity = { 0.0f,0.0f,0.0f };
			Quaternion orientation = Quaternion{};
			Vector3 lookAt = { 0.0f,0.0f,0.0f };
		};

	public:
		Camera(D3D12RHI& gfx, std::string name, Transform transform, bool tethered = false) noexcept(!IS_DEBUG);
		Matrix GetViewMatrix() const noexcept(!IS_DEBUG);
		Matrix GetProjectionMatrix() const noexcept(!IS_DEBUG);
		void SetViewMatrix() noexcept(!IS_DEBUG);
		void SetProjectionMatrix() noexcept(!IS_DEBUG);
		void SpawnControlWidgets(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		void Reset(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		void Rotate(float dx, float dy) noexcept(!IS_DEBUG);
		void Translate(Vector3 translation, float dt) noexcept(!IS_DEBUG);
		Vector3 GetPos() const noexcept(!IS_DEBUG);
		void SetPos(const Vector3& pos) noexcept(!IS_DEBUG);
		const std::string& GetName() const noexcept(!IS_DEBUG);
		void Submit(RenderGraph& renderGraph) const;

	private:
		Matrix m_viewMatrix{};
		Matrix m_projectionMatrix{};
		bool m_tethered;
		std::string m_name;
		float m_fov = XMConvertToRadians(45.0f);
		float m_travelSpeed = 12.0f;
		float m_rotationSpeed = 0.004f;
		CameraProjection::Projection m_projection;
		CameraProjection::Projection m_homeProjection;
		Transform m_transform;
		Transform m_homeTransform;
		bool m_enableCameraIndicator = false;
		bool m_enableCameraProjection = false;
		CameraProjection m_cameraProjection;
		CameraIndicator m_cameraIndicator;
	};
}