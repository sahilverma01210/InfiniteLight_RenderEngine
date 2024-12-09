#pragma once
#include "RenderQueuePass.h"

namespace Renderer
{
	class D3D12RHI;

	class OutlineMaskGenerationPass : public RenderQueuePass
	{
	public:
		OutlineMaskGenerationPass(D3D12RHI& gfx, std::string name)
			:
			RenderQueuePass(std::move(name))
		{
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", depthStencil));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", depthStencil));
		}
	};
}