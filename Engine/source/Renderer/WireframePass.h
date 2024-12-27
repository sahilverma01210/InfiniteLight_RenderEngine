#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class WireframePass : public RenderPass
	{
	public:
		WireframePass(D3D12RHI& gfx, std::string name)
			:
			RenderPass(std::move(name))
		{
			RegisterSink(DirectBufferBucketSink<RenderTarget>::Make("renderTarget", m_renderTargetVector));
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", m_depthStencil));

			RegisterSource(DirectBufferBucketSource<RenderTarget>::Make("renderTarget", m_renderTargetVector));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", m_depthStencil));
		}
	};
}