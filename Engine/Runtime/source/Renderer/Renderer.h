#pragma once

#include "../Common/ILMath.h"
#include "../ImGUI/ImGUI_Manager.h"

#include "D3D12RHI.h"
#include "Camera.h"
#include "PointLight.h"
#include "Box.h"
#include "Cylinder.h"
#include "Pyramid.h"
#include "TexturedBox.h"

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
			const XMFLOAT3 mat = { cdist(rng),cdist(rng),cdist(rng) };
			
			switch (sdist(rng))
			{
			case 0:
				return std::make_unique<Box>(
					gfx, rng, adist, ddist,
					odist, rdist, bdist, mat
				);
			case 1:
				return std::make_unique<Cylinder>(
					gfx, rng, adist, ddist, odist,
					rdist, bdist, tdist
				);
			case 2:
				return std::make_unique<Pyramid>(
					gfx, rng, adist, ddist, odist,
					rdist, tdist
				);
			case 3:
				return std::make_unique<TexturedBox>(
					gfx, rng, adist, ddist,
					odist, rdist
				);
			default:
				assert(false && "impossible drawable option in factory");
				return {};
			}
		}
	private:
		D3D12RHI& gfx;
		std::mt19937 rng{ std::random_device{}() };
		std::uniform_int_distribution<int> sdist{ 0,3 };
		std::uniform_real_distribution<float> adist{ 0.0f,PI * 2.0f };
		std::uniform_real_distribution<float> ddist{ 0.0f,PI * 0.5f };
		std::uniform_real_distribution<float> odist{ 0.0f,PI * 0.08f };
		std::uniform_real_distribution<float> rdist{ 6.0f,20.0f };
		std::uniform_real_distribution<float> bdist{ 0.4f,3.0f };
		std::uniform_real_distribution<float> cdist{ 0.0f,1.0f };
		std::uniform_int_distribution<int> tdist{ 3,30 };
	};

	void init(UINT width, UINT height, HWND hWnd, HINSTANCE hInstance, bool useWarpDevice);

	void update(float angle);

	void destroy();
}