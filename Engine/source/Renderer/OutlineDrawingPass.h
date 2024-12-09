#pragma once
#include "RenderQueuePass.h"

/*
* Todo (Optional): Configure this Pass and use it in Render Graph. Modify Material outlineDraw Step PipelineDesc for the same.
*/

namespace Renderer
{
	class D3D12RHI;

	class OutlineDrawingPass : public RenderQueuePass
	{
	public:
		OutlineDrawingPass(D3D12RHI& gfx, std::string name)
			:
			RenderQueuePass(std::move(name))
		{
			RegisterSink(DirectBufferBucketSink<RenderTarget>::Make("renderTarget", renderTargetVector));
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", depthStencil));
			RegisterSource(DirectBufferBucketSource<RenderTarget>::Make("renderTarget", renderTargetVector));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", depthStencil));
			
			//AddBind(Stencil::Resolve(gfx, Stencil::Mode::Mask));
			//AddBind(Rasterizer::Resolve(gfx, false));
		}
	};
}