#include "HorizontalBlurPass.h"
#include "RenderTarget.h"
#include "Sink.h"
#include "Source.h"
#include "ConstantBuffer.h"

namespace Renderer
{
	HorizontalBlurPass::HorizontalBlurPass(std::string name, D3D12RHI& gfx, unsigned int fullWidth, unsigned int fullHeight)
		:
		FullscreenPass(std::move(name), gfx)
	{
		renderTargetVector.push_back(std::make_shared<RenderTarget>(gfx, fullWidth, fullHeight));

		ID3DBlob* pixelShader;

		D3DCompileFromFile(gfx.GetAssetFullPath(L"BlurOutline_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &pixelShader, nullptr);

		pipelineDesc.pixelShader = pixelShader;
		pipelineDesc.blending = false;
		pipelineDesc.depthStencilMode = Mode::Off;
		pipelineDesc.samplerFilterType = SamplerFilterType::Point;
		pipelineDesc.reflect = true;

		rootSignBindable = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
		psoBindable = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
		srvBindable = std::move(std::make_unique<ShaderResourceView>(gfx, 2, 1));

		AddBindSink<ConstantBuffer>("kernel");

		RegisterSink(DirectBindableSink<RenderTarget>::Make("scratchIn", blurTarget));
		RegisterSink(DirectBindableSink<ConstantBuffer>::Make("direction", direction));
		
		// the renderTarget is internally sourced and then exported as a Bindable
		RegisterSource(DirectBindableSource<RenderTarget>::Make("scratchOut", renderTargetVector[0]));
	}

	// this override is necessary because we cannot (yet) link input bindables directly into
	// the container of bindables (mainly because vector growth buggers references)
	void HorizontalBlurPass::Execute(D3D12RHI& gfx) const noexcept
	{
		auto buf = direction->GetBuffer();
		buf["isHorizontal"] = true;
		direction->SetBuffer(buf);

		rootSignBindable->Bind(gfx);
		psoBindable->Bind(gfx);

		gfx.TransitionResource(blurTarget->GetBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		srvBindable->AddResource(gfx, 0, blurTarget->GetBuffer());
		srvBindable->Bind(gfx);

		direction->Bind(gfx);
		FullscreenPass::Execute(gfx);
		gfx.TransitionResource(blurTarget->GetBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
}