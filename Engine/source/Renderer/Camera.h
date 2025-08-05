#pragma once
#include "../Common/ImGUI_Includes.h"

#include "RenderMath.h"

namespace Renderer
{
	class Camera
	{
	public:
		struct State
		{
			bool rotate = false;
			bool move = false;
			bool project = false;
			bool enableCameraIndicator = false;
			bool enableCameraProjection = false;
		};
		struct Transform
		{
			Vector3 position = { 0.0f,0.0f,0.0f }; // X, Y, Z
			Vector3 rotation = { 0.0f,0.0f,0.0f }; // Pitch, Yaw, Roll
			Vector3 velocity = { 0.0f,0.0f,0.0f };
			Quaternion orientation = Quaternion{};
			Vector3 lookAt = { 0.0f,0.0f,0.0f };
		};
		struct Projection
		{
			float width;
			float height;
			float nearZ;
			float farZ;
		};
		struct CameraData
		{
			Matrix viewMat;
			Matrix projectionMat;
			Matrix inverseViewMat;
			Matrix inverseProjectionMat;
			Matrix inverseViewProjectionMat;
			Vector3 position;
		};

	public:
		Camera(std::string name, Transform transform) noexcept(!IS_DEBUG);
		void Update() noexcept(!IS_DEBUG);
		Matrix GetViewMatrix() const noexcept(!IS_DEBUG);
		Matrix GetProjectionMatrix() const noexcept(!IS_DEBUG);
		void SetViewMatrix() noexcept(!IS_DEBUG);
		void SetProjectionMatrix() noexcept(!IS_DEBUG);
		void SpawnControlWidgets() noexcept(!IS_DEBUG);
		void ResetParams() noexcept(!IS_DEBUG);
		void ResetSate() noexcept(!IS_DEBUG);
		void Rotate(Vector2 rotation) noexcept(!IS_DEBUG);
		void Translate(Vector3 translation) noexcept(!IS_DEBUG);
		Transform& GetTransform() noexcept(!IS_DEBUG) { return m_transform; }
		Projection& GetProjection() noexcept(!IS_DEBUG) { return m_projection; }
		const std::string& GetName() const noexcept(!IS_DEBUG) { return m_name; }
		CameraData& GetCameraData() noexcept(!IS_DEBUG) { return m_cameraData; }
		State& GetState() noexcept(!IS_DEBUG) { return m_state; }
		void SetInvertProjection(bool invert) noexcept(!IS_DEBUG) { m_invertProjection = invert; }

	private:
		bool m_invertProjection = false;
		State m_state;
		Matrix m_viewMatrix{};
		Matrix m_projectionMatrix{};
		std::string m_name;
		float m_fov = XMConvertToRadians(45.0f);
		float m_travelSpeed = 12.0f;
		float m_rotationSpeed = 0.004f;
		Projection m_projection;
		Projection m_homeProjection;
		Transform m_transform;
		Transform m_homeTransform;
		CameraData m_cameraData{};
	};
}