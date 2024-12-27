#pragma once
#include "RenderMath.h"
#include "CameraIndicator.h"
#include "CameraProjection.h"
#include "UIManager.h"

namespace Renderer
{
	class Camera
	{
	public:
		struct Transform
		{
			XMFLOAT3 position = { 0.0f,0.0f,0.0f };
			float pitch = 0.0f;
			float yaw = 0.0f;
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
		void LinkTechniques(RenderGraph& rg);
		void Submit(size_t channel) const;

	private:
		bool m_tethered;
		std::string m_name;
		float m_travelSpeed = 12.0f;
		float m_rotationSpeed = 0.004f;
		Transform m_transform;
		Transform m_homeTransform;
		bool m_enableCameraIndicator = false;
		bool m_enableFrustumIndicator = false;
		CameraProjection m_projection;
		CameraIndicator m_indicator;
	};
}