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
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", m_depthStencil));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", m_depthStencil));
		}
	};
}