#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class OutlineMaskGenerationPass : public RenderPass
	{
	public:
		OutlineMaskGenerationPass(D3D12RHI& gfx, std::string name)
			:
			RenderPass(std::move(name))
		{
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencilBuffer", m_depthStencil));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencilBuffer", m_depthStencil));
		}
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override
		{
			RenderPass::Execute(gfx);
		}
	};
}