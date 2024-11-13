#include "HorizontalBlurPass.h"
//#include "PixelShader.h"
#include "RenderTarget.h"
#include "Sink.h"
#include "Source.h"
//#include "Blender.h"
//#include "Sampler.h"
#include "ConstantBuffer.h"

namespace Renderer
{
	HorizontalBlurPass::HorizontalBlurPass(std::string name, D3D12RHI& gfx, unsigned int fullWidth, unsigned int fullHeight)
		:
		FullscreenPass(std::move(name), gfx)
	{
		//AddBind(PixelShader::Resolve(gfx, "BlurOutline_PS.cso"));
		//AddBind(Blender::Resolve(gfx, false));
		//AddBind(Sampler::Resolve(gfx, Sampler::Type::Point, true));

		//AddBindSink<RenderTarget>("scratchIn");
		//AddBindSink<ConstantBuffer>("kernel");
		//RegisterSink(DirectBindableSink<CachingPixelConstantBufferEx>::Make("direction", direction));

		//// the renderTarget is internally sourced and then exported as a Bindable
		//renderTarget = std::make_shared<ShaderInputRenderTarget>(gfx, fullWidth / 2, fullHeight / 2, 0u);
		//RegisterSource(DirectBindableSource<RenderTarget>::Make("scratchOut", renderTarget));
	}

	// this override is necessary because we cannot (yet) link input bindables directly into
	// the container of bindables (mainly because vector growth buggers references)
	void HorizontalBlurPass::Execute(D3D12RHI& gfx) const noexcept
	{
		//auto buf = direction->GetBuffer();
		//buf["isHorizontal"] = true;
		//direction->SetBuffer(buf);

		//direction->Bind(gfx);
		//FullscreenPass::Execute(gfx);
	}
}