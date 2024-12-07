#pragma once
#include "Bindable.h"
#include "BindableCodex.h"

namespace Renderer
{
	class Viewport : public Bindable
	{
	public:
		Viewport(D3D12RHI& gfx)
			:
			Viewport(gfx, (float)gfx.GetWidth(), (float)gfx.GetHeight())
		{
		}
		Viewport(D3D12RHI& gfx, float width, float height)
		{
			vp.Width = width;
			vp.Height = height;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0.0f;
			vp.TopLeftY = 0.0f;
		}
		void Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			GetCommandList(gfx)->RSSetViewports(1u, &vp);
		}

	private:
		D3D12_VIEWPORT vp{};
	};
}