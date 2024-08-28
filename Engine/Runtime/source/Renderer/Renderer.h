#pragma once
#include "D3D12RHI.h"
#include "../Common/ILMath.h"
#include "Camera.h"
#include "Box.h"
#include "Melon.h"
#include "Pyramid.h"
#include <algorithm>

namespace Renderer
{
	class Factory
	{
	public:
		Factory(D3D12RHI& gfx)
			:
			gfx(gfx)
		{}
		std::unique_ptr<Drawable> operator()()
		{
			switch (typedist(rng))
			{
			case 0:
				return std::make_unique<Pyramid>(
					gfx, rng, adist, ddist,
					odist, rdist
				);
			case 1:
				return std::make_unique<BoxA>(
					gfx, rng, adist, ddist,
					odist, rdist, bdist
				);
			case 2:
				return std::make_unique<BoxB>(
					gfx, rng, adist, ddist,
					odist, rdist, bdist
				);
			case 3:
				return std::make_unique<Melon>(
					gfx, rng, adist, ddist,
					odist, rdist, longdist, latdist
				);
			default:
				assert(false && "bad drawable type in factory");
				return {};
			}
		}
	private:
		D3D12RHI& gfx;
		std::mt19937 rng{ std::random_device{}() };
		std::uniform_real_distribution<float> adist{ 0.0f,PI * 2.0f };
		std::uniform_real_distribution<float> ddist{ 0.0f,PI * 0.5f };
		std::uniform_real_distribution<float> odist{ 0.0f,PI * 0.08f };
		std::uniform_real_distribution<float> rdist{ 6.0f,20.0f };
		std::uniform_real_distribution<float> bdist{ 0.4f,3.0f };
		std::uniform_int_distribution<int> latdist{ 5,20 };
		std::uniform_int_distribution<int> longdist{ 10,40 };
		std::uniform_int_distribution<int> typedist{ 0,3 };
	};

	void init(UINT width, UINT height, HWND hWnd, HINSTANCE hInstance, bool useWarpDevice);

	void update(float angle);

	void destroy();
}