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
			XMFLOAT3 position = { 0.0f,0.0f,0.0f }; // X, Y, Z
			XMFLOAT3 rotation = { 0.0f,0.0f,0.0f }; // Pitch, Yaw, Roll
		};

	public:
		Camera(D3D12RHI& gfx, std::string name, Transform transform, bool tethered = false) noexcept(!IS_DEBUG);
		void Update(bool has360View = false, UINT direction = 0) const noexcept(!IS_DEBUG);
		XMMATRIX GetCameraMatrix() const noexcept(!IS_DEBUG);
		XMMATRIX GetProjectionMatrix() const noexcept(!IS_DEBUG);
		XMMATRIX Get360CameraMatrix(UINT directionIndex) const noexcept(!IS_DEBUG);
		XMMATRIX Get360ProjectionMatrix() const noexcept(!IS_DEBUG);
		void SpawnControlWidgets(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		void Reset(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		void Rotate(float dx, float dy) noexcept(!IS_DEBUG);
		void Translate(XMFLOAT3 translation) noexcept(!IS_DEBUG);
		XMFLOAT3 GetPos() const noexcept(!IS_DEBUG);
		void SetPos(const XMFLOAT3& pos) noexcept(!IS_DEBUG);
		const std::string& GetName() const noexcept(!IS_DEBUG);
		void Submit(RenderGraph& renderGraph) const;

	private:
		bool m_tethered;
		std::string m_name;
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