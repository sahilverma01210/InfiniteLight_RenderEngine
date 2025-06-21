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
			m_renderTargets.resize(1);
			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.GetResourcePtr(RenderGraph::m_frameResourceHandles["Depth_Stencil"]));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			m_renderTargets[0] = gfx.GetResourcePtr(gfx.GetCurrentBackBufferIndex());
			gfx.ClearResource(gfx.GetCurrentBackBufferIndex());
			gfx.ClearResource(RenderGraph::m_frameResourceHandles["Depth_Stencil"]);
		}
	};
}