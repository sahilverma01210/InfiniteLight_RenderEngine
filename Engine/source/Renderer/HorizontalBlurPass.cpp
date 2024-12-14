#include "HorizontalBlurPass.h"

namespace Renderer
{
	HorizontalBlurPass::HorizontalBlurPass(std::string name, D3D12RHI& gfx, unsigned int fullWidth, unsigned int fullHeight)
		:
		FullscreenPass(std::move(name), gfx)
	{
		m_renderTargetVector.push_back(std::make_shared<RenderTarget>(gfx, fullWidth, fullHeight));

		ID3DBlob* pixelShader;

		D3DCompileFromFile(GetAssetFullPath(L"BlurOutline_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", 0, 0, &pixelShader, nullptr);


		CD3DX12_STATIC_SAMPLER_DESC* samplers = new CD3DX12_STATIC_SAMPLER_DESC[1];

		// define static sampler 
		CD3DX12_STATIC_SAMPLER_DESC staticSampler{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
		staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		staticSampler.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
		staticSampler.MipLODBias = 0.0f;
		staticSampler.MinLOD = 0.0f;
		staticSampler.MaxLOD = D3D12_FLOAT32_MAX;

		samplers[0] = staticSampler;

		m_pipelineDesc.numSamplers = 1;
		m_pipelineDesc.samplers = samplers;
		m_pipelineDesc.pixelShader = pixelShader;

		m_rootSignBindable = std::move(std::make_unique<RootSignature>(gfx, m_pipelineDesc));
		m_psoBindable = std::move(std::make_unique<PipelineState>(gfx, m_pipelineDesc));
		m_srvBindable = std::move(std::make_unique<ShaderResourceView>(gfx, 2, 1));

		AddBindSink<ConstantBuffer>("kernel");

		RegisterSink(DirectBindableSink<RenderTarget>::Make("scratchIn", m_blurTarget));
		RegisterSink(DirectBindableSink<ConstantBuffer>::Make("direction", m_direction));
		
		// the renderTarget is internally sourced and then exported as a Bindable
		RegisterSource(DirectBindableSource<RenderTarget>::Make("scratchOut", m_renderTargetVector[0]));
	}

	// this override is necessary because we cannot (yet) link input bindables directly into
	// the container of bindables (mainly because vector growth buggers references)
	void HorizontalBlurPass::Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		auto buf = m_direction->GetBuffer();
		buf["isHorizontal"] = true;
		m_direction->SetBuffer(buf);

		m_rootSignBindable->Bind(gfx);
		m_psoBindable->Bind(gfx);

		gfx.TransitionResource(m_blurTarget->GetBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		m_srvBindable->AddTextureResource(gfx, 0, m_blurTarget->GetBuffer());
		m_srvBindable->Bind(gfx);

		m_direction->Bind(gfx);
		FullscreenPass::Execute(gfx);
		gfx.TransitionResource(m_blurTarget->GetBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
}