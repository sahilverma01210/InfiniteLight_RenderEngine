#include "VerticalBlurPass.h"
#include "Sink.h"
#include "Source.h"

namespace Renderer
{
	VerticalBlurPass::VerticalBlurPass(std::string name, D3D12RHI& gfx)
		:
		FullscreenPass(std::move(name), gfx)
	{
		ID3DBlob* pixelShader;

		D3DCompileFromFile(gfx.GetAssetFullPath(L"BlurOutline_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &pixelShader, nullptr);

		pipelineDesc.pixelShader = pixelShader;
		pipelineDesc.blending = true;
		pipelineDesc.depthStencilMode = Mode::Mask;
		pipelineDesc.samplerFilterType = SamplerFilterType::Bilinear;
		pipelineDesc.reflect = true;

		rootSignBindable = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
		psoBindable = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
		srvBindable = std::move(std::make_unique<ShaderResourceView>(gfx, 2, 1));

		AddBindSink<ConstantBuffer>("kernel");

		RegisterSink(DirectBindableSink<RenderTarget>::Make("scratchIn", blurTarget));
		RegisterSink(DirectBindableSink<ConstantBuffer>::Make("direction", direction));

		RegisterSink(DirectBufferBucketSink<RenderTarget>::Make("renderTarget", renderTargetVector));
		RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", depthStencil));
		
		RegisterSource(DirectBufferBucketSource<RenderTarget>::Make("renderTarget", renderTargetVector));
		RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", depthStencil));
	}

	// see the note on HorizontalBlurPass::Execute
	void VerticalBlurPass::Execute(D3D12RHI& gfx) const noexcept
	{
		auto buf = direction->GetBuffer();
		buf["isHorizontal"] = false;
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