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
		m_viewPort.Width = width;
		m_viewPort.Height = height;
		m_viewPort.MinDepth = 0.0f;
		m_viewPort.MaxDepth = 1.0f;
		m_viewPort.TopLeftX = 0.0f;
		m_viewPort.TopLeftY = 0.0f;
	}

	void Viewport::Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		INFOMAN_NOHR(gfx);
		D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->RSSetViewports(1u, &m_viewPort));
	}
}