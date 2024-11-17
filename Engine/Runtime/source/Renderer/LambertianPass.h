#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
#include "Sink.h"
#include "Source.h"

class D3D12RHI;

namespace Renderer
{
	class LambertianPass : public RenderQueuePass
	{
	public:
		LambertianPass(D3D12RHI& gfx, std::string name)
			:
			RenderQueuePass(std::move(name))
		{
			RegisterSink(DirectBufferBucketSink<RenderTarget>::Make("renderTarget", renderTargetVector));
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", depthStencil));

			RegisterSource(DirectBufferBucketSource<RenderTarget>::Make("renderTarget", renderTargetVector));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", depthStencil));
		}
	};
}