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
		Camera(D3D12RHI& gfx, std::string name, XMFLOAT3 homePos = { 0.0f,0.0f,0.0f }, float homePitch = 0.0f, float homeYaw = 0.0f, bool tethered = false) noexcept(!IS_DEBUG);
		XMMATRIX GetMatrix() const noexcept(!IS_DEBUG);
		XMMATRIX GetProjection() const noexcept(!IS_DEBUG);
		void SpawnControlWidgets(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		void Update(D3D12RHI& gfx) const;
		void Reset(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		void Rotate(float dx, float dy) noexcept(!IS_DEBUG);
		void Translate(XMFLOAT3 translation) noexcept(!IS_DEBUG);
		XMFLOAT3 GetPos() const noexcept(!IS_DEBUG);
		void SetPos(const XMFLOAT3& pos) noexcept(!IS_DEBUG);
		const std::string& GetName() const noexcept(!IS_DEBUG);
		void LinkTechniques(RenderGraph& rg);
		void Submit(size_t channels) const;

	private:
		bool m_tethered;
		std::string m_name;
		XMFLOAT3 m_pos;
		XMFLOAT3 m_homePos;
		float m_pitch;
		float m_homePitch;
		float m_yaw;
		float m_homeYaw;
		CameraProjection m_projection;
		CameraIndicator m_indicator;
		bool m_enableCameraIndicator = false;
		bool m_enableFrustumIndicator = false;
		static constexpr float m_travelSpeed = 12.0f;
		static constexpr float m_rotationSpeed = 0.004f;
	};
}