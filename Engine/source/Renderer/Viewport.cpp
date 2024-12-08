#include "Viewport.h"

namespace Renderer
{
	Viewport::Viewport(D3D12RHI& gfx)
		:
	Viewport(gfx, (float)gfx.GetWidth(), (float)gfx.GetHeight())
	{
	}

	Viewport::Viewport(D3D12RHI& gfx, float width, float height)
	{
		vp.Width = width;
		vp.Height = height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
	}

	void Viewport::Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		INFOMAN_NOHR(gfx);
		D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->RSSetViewports(1u, &vp));
	}
}