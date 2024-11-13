#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
//#include "PixelShader.h"
//#include "VertexShader.h"
//#include "Stencil.h"
//#include "Rasterizer.h"

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
			//RegisterSink(DirectBufferSink<RenderTarget>::Make("renderTarget", renderTarget));
			//RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", depthStencil));
			//RegisterSource(DirectBufferSource<RenderTarget>::Make("renderTarget", renderTarget));
			//RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", depthStencil));
			//AddBind(VertexShader::Resolve(gfx, "Solid_VS.cso"));
			//AddBind(PixelShader::Resolve(gfx, "Solid_PS.cso"));
			//AddBind(Stencil::Resolve(gfx, Stencil::Mode::Mask));
			//AddBind(Rasterizer::Resolve(gfx, false));
		}
	};
}