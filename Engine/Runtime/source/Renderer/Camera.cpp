#include "Camera.h"

namespace Renderer
{
	XMMATRIX Camera::GetMatrix() const noexcept
	{
		const auto pos = XMVector3Transform(
			XMVectorSet(0.0f, 0.0f, -r, 0.0f),
			XMMatrixRotationRollPitchYaw(phi, -theta, 0.0f)
		);
		return XMMatrixLookAtLH(
			pos, XMVectorZero(),
			XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
		) * XMMatrixRotationRollPitchYaw(
			pitch, -yaw, roll
		);
	}
	void Camera::SpawnControlWindow() noexcept
	{
	}
	void Camera::Reset() noexcept
	{
		r = 20.0f;
		theta = 0.0f;
		phi = 0.0f;
		pitch = 0.0f;
		yaw = 0.0f;
		roll = 0.0f;
	}
}