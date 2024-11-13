#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
//#include "NullPixelShader.h"
//#include "VertexShader.h"
//#include "Stencil.h"
//#include "Rasterizer.h"

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
			//AddBind(VertexShader::Resolve(gfx, "Solid_VS.cso"));
			//AddBind(NullPixelShader::Resolve(gfx));
			//AddBind(Stencil::Resolve(gfx, Stencil::Mode::Write));
			//AddBind(Rasterizer::Resolve(gfx, false));
		}
	};
}