#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
//#include "PixelShader.h"
//#include "VertexShader.h"
//#include "Stencil.h"
//#include "Rasterizer.h"
#include "Source.h"
#include "RenderTarget.h"
//#include "Blender.h"

namespace Renderer
{
	class D3D12RHI;

	class BlurOutlineDrawingPass : public RenderQueuePass
	{
	public:
		BlurOutlineDrawingPass(D3D12RHI& gfx, std::string name, unsigned int fullWidth, unsigned int fullHeight)
			:
			RenderQueuePass(std::move(name))
		{
			//renderTarget = std::make_unique<ShaderInputRenderTarget>(gfx, fullWidth / 2, fullHeight / 2, 0);
			//AddBind(VertexShader::Resolve(gfx, "Solid_VS.cso"));
			//AddBind(PixelShader::Resolve(gfx, "Solid_PS.cso"));
			//AddBind(Stencil::Resolve(gfx, Stencil::Mode::Mask));
			//AddBind(Blender::Resolve(gfx, false));
			//RegisterSource(DirectBindableSource<RenderTarget>::Make("scratchOut", renderTarget));
		}
		void Execute(D3D12RHI& gfx) const noexcept override
		{
			//renderTarget->Clear(gfx);
			//RenderQueuePass::Execute(gfx);
		}
	};
}