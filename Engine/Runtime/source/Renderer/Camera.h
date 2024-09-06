#pragma once
#include "D3D12RHI.h"

namespace Renderer
{
	class Camera
	{
	public:
		XMMATRIX GetMatrix() const noexcept;
		bool SpawnControlWindow() noexcept;
		void Reset() noexcept;

		bool m_imGUIwndOpen = true;
	private:
		float r = 20.0f;
		float theta = 0.0f;
		float phi = 0.0f;
		float pitch = 0.0f;
		float yaw = 0.0f;
		float roll = 0.0f;
	};
}