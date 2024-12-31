#pragma once
#include "RenderPass.h"
#include "Camera.h"
#include "Shapes.h"

namespace Renderer
{
	class SkyboxPass : public RenderPass
	{
	public:
		SkyboxPass(D3D12RHI& gfx, std::string name)
			:
			RenderPass(std::move(name))
		{
			RegisterSink(DirectBufferBucketSink<RenderTarget>::Make("renderTargetBuffers", m_renderTargetVector));
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencilBuffer", m_depthStencil));
			RegisterSource(DirectBufferBucketSource<RenderTarget>::Make("renderTargetBuffers", m_renderTargetVector));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencilBuffer", m_depthStencil));
		}
	};
}