#pragma once
#include "../Common/ILMath.h"

#include "D3D12RHI.h"
#include "UIManager.h"

using namespace Common;

namespace Renderer
{
	class Camera
	{
	public:
		Camera(D3D12RHI& gfx) noexcept;
		XMMATRIX GetMatrix() const noexcept;
		bool SpawnControlWindow(D3D12RHI& gfx) noexcept;
		void Update(D3D12RHI& gfx) noexcept;
		void Reset(D3D12RHI& gfx) noexcept;
		void Rotate(float dx, float dy) noexcept;
		void Translate(XMFLOAT3 translation) noexcept;
		bool m_imGUIwndOpen = true;
	private:
		XMFLOAT3 pos;
		float pitch;
		float yaw;
		static constexpr float travelSpeed = 12.0f;
		static constexpr float rotationSpeed = 0.004f;
	};
}