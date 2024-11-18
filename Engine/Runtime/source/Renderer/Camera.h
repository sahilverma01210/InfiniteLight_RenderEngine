#pragma once
#include "../Common/ILMath.h"

#include <DirectXMath.h>
#include <string>
#include "Projection.h"
#include "CameraIndicator.h"

using namespace Common;

namespace Renderer
{
	class D3D12RHI;
	class RenderGraph;

	class Camera
	{
	public:
		Camera(D3D12RHI& gfx, std::string name, XMFLOAT3 homePos = { 0.0f,0.0f,0.0f }, float homePitch = 0.0f, float homeYaw = 0.0f) noexcept;
		XMMATRIX GetMatrix() const noexcept;
		void SpawnControlWidgets(D3D12RHI& gfx) noexcept;
		void Update(D3D12RHI& gfx) noexcept;
		void Reset(D3D12RHI& gfx) noexcept;
		void Rotate(float dx, float dy) noexcept;
		void Translate(XMFLOAT3 translation) noexcept;
		const std::string& GetName() const noexcept;
		void LinkTechniques(RenderGraph& rg);
		void Submit() const;
	private:
		std::string name;
		XMFLOAT3 homePos;
		float homePitch;
		float homeYaw;
		XMFLOAT3 pos;
		float pitch;
		float yaw;
		static constexpr float travelSpeed = 12.0f;
		static constexpr float rotationSpeed = 0.004f;
		bool enableCameraIndicator = true;
		bool enableFrustumIndicator = true;
		Projection proj;
		CameraIndicator indicator;
	};
}