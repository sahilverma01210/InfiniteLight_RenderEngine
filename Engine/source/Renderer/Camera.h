#pragma once
#include "../Common/ILMath.h"

#include "Projection.h"
#include "CameraIndicator.h"
#include "D3D12RHI.h"
#include "UIManager.h"

using namespace Common;

namespace Renderer
{
	class Camera
	{
	public:
		Camera(D3D12RHI& gfx, std::string name, XMFLOAT3 homePos = { 0.0f,0.0f,0.0f }, float homePitch = 0.0f, float homeYaw = 0.0f, bool tethered = false) noexcept;
		XMMATRIX GetMatrix() const noexcept;
		XMMATRIX GetProjection() const noexcept;
		void SpawnControlWidgets(D3D12RHI& gfx) noexcept;
		void Update(D3D12RHI& gfx) const;
		void Reset(D3D12RHI& gfx) noexcept;
		void Rotate(float dx, float dy) noexcept;
		void Translate(XMFLOAT3 translation) noexcept;
		XMFLOAT3 GetPos() const noexcept;
		void SetPos(const XMFLOAT3& pos) noexcept;
		const std::string& GetName() const noexcept;
		void LinkTechniques(RenderGraph& rg);
		void Submit(size_t channels) const;

	private:
		bool tethered;
		std::string name;
		XMFLOAT3 homePos;
		float homePitch;
		float homeYaw;
		XMFLOAT3 pos;
		float pitch;
		float yaw;
		static constexpr float travelSpeed = 12.0f;
		static constexpr float rotationSpeed = 0.004f;
		bool enableCameraIndicator = false;
		bool enableFrustumIndicator = false;
		Projection proj;
		CameraIndicator indicator;
	};
}