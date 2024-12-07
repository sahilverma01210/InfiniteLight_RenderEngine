#include "VerticalBlurPass.h"

namespace Renderer
{
	VerticalBlurPass::VerticalBlurPass(std::string name, D3D12RHI& gfx)
		:
		FullscreenPass(std::move(name), gfx)
	{
		ID3DBlob* pixelShader;

		D3DCompileFromFile(gfx.GetAssetFullPath(L"BlurOutline_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", 0, 0, &pixelShader, nullptr);


		CD3DX12_STATIC_SAMPLER_DESC* samplers = new CD3DX12_STATIC_SAMPLER_DESC[1];

		// define static sampler 
		CD3DX12_STATIC_SAMPLER_DESC staticSampler{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
		staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		staticSampler.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
		staticSampler.MipLODBias = 0.0f;
		staticSampler.MinLOD = 0.0f;
		staticSampler.MaxLOD = D3D12_FLOAT32_MAX;

		samplers[0] = staticSampler;

		pipelineDesc.numSamplers = 1;
		pipelineDesc.samplers = samplers;
		pipelineDesc.blending = true;
		pipelineDesc.depthStencilMode = Mode::Mask;
		pipelineDesc.pixelShader = pixelShader;

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
	void VerticalBlurPass::Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		auto buf = direction->GetBuffer();
		buf["isHorizontal"] = false;
		direction->SetBuffer(buf);
		
		rootSignBindable->Bind(gfx);
		psoBindable->Bind(gfx);

		gfx.TransitionResource(blurTarget->GetBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		srvBindable->AddTextureResource(gfx, 0, blurTarget->GetBuffer());
		srvBindable->Bind(gfx);

		direction->Bind(gfx);
		FullscreenPass::Execute(gfx);
		gfx.TransitionResource(blurTarget->GetBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
}