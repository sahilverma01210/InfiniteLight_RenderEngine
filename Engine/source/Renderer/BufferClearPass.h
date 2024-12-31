#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class BufferClearPass : public RenderPass
	{
	public:
		BufferClearPass(std::string name)
			:
			RenderPass(std::move(name))
		{
			RegisterSink(DirectBufferBucketSink<RenderTarget>::Make("renderTargetBuffers", m_renderTargetVector));
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencilBuffer", m_depthStencil));
			RegisterSource(DirectBufferBucketSource<RenderTarget>::Make("renderTargetBuffers", m_renderTargetVector));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencilBuffer", m_depthStencil));
		}
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override
		{
			m_renderTargetVector[gfx.GetCurrentBackBufferIndex()]->Clear(gfx);
			m_depthStencil->Clear(gfx);
		}
	};
}