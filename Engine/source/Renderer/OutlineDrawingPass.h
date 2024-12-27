#pragma once
#include "RenderPass.h"

/*
* Todo (Optional): Configure this Pass and use it in Render Graph. Modify Material outlineDraw Step PipelineDesc for the same.
*/

namespace Renderer
{
	class OutlineDrawingPass : public RenderPass
	{
	public:
		OutlineDrawingPass(D3D12RHI& gfx, std::string name)
			:
			RenderPass(std::move(name))
		{
			RegisterSink(DirectBufferBucketSink<RenderTarget>::Make("renderTarget", m_renderTargetVector));
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", m_depthStencil));
			RegisterSource(DirectBufferBucketSource<RenderTarget>::Make("renderTarget", m_renderTargetVector));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", m_depthStencil));
			
			//AddBind(Stencil::Resolve(gfx, Stencil::Mode::Mask));
			//AddBind(Rasterizer::Resolve(gfx, false));
		}
	};
}