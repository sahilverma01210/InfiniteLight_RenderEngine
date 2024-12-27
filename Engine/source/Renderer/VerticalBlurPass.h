#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class VerticalBlurPass : public RenderPass
	{
	public:
		VerticalBlurPass(D3D12RHI& gfx, std::string name)
			:
			RenderPass(std::move(name))
		{
			RegisterSink(DirectBufferBucketSink<RenderTarget>::Make("renderTarget", m_renderTargetVector));
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", m_depthStencil));

			RegisterSource(DirectBufferBucketSource<RenderTarget>::Make("renderTarget", m_renderTargetVector));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", m_depthStencil));
		}

		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override
		{
			ID3D12Resource* blurTargetBuffer = gfx.GetRenderTargetBuffers().back().Get();
			gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			RenderPass::Execute(gfx);

			gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
	};
}