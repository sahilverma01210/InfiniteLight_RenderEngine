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
			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.GetResourcePtr(2));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			m_renderTarget = std::dynamic_pointer_cast<RenderTarget>(gfx.GetResourcePtr(gfx.GetCurrentBackBufferIndex()));
			m_renderTarget->Clear(gfx);
			m_depthStencil->Clear(gfx);
		}
	};
}