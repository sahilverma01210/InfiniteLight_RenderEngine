#pragma once
#include "Object.h"
#include <random>

namespace Renderer
{
	class Box : public Object<Box>
	{
	public:
		Box(D3D12RHI& gfx, std::mt19937& rng,
			std::uniform_real_distribution<float>& adist,
			std::uniform_real_distribution<float>& ddist,
			std::uniform_real_distribution<float>& odist,
			std::uniform_real_distribution<float>& rdist,
			std::uniform_real_distribution<float>& bdist);
		void Update(float dt) noexcept override;
		XMMATRIX GetTransformXM() const noexcept override;
	private:
		// positional
		float r;
		float roll = 0.0f;
		float pitch = 0.0f;
		float yaw = 0.0f;
		float theta;
		float phi;
		float chi;
		// speed (delta/s)
		float droll;
		float dpitch;
		float dyaw;
		float dtheta;
		float dphi;
		float dchi;
		// model transform
		XMFLOAT3X3 mt;
	};
}