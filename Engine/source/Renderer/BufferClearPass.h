#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class BufferClearPass : public RenderPass
	{
	public:
		BufferClearPass(D3D12RHI& gfx, std::string name)
			:
			RenderPass(std::move(name))
		{
			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.m_textureManager.GetTexturePtr(3));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			m_renderTarget = std::dynamic_pointer_cast<RenderTarget>(gfx.m_textureManager.GetTexturePtr(gfx.GetCurrentBackBufferIndex() + 1));
			m_renderTarget->Clear(gfx);
			m_depthStencil->Clear(gfx);
		}
	};
}