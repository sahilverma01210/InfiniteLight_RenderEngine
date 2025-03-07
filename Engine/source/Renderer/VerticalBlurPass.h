#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class VerticalBlurPass : public RenderPass
	{
	public:
		VerticalBlurPass(D3D12RHI& gfx, std::string name)
			:
			RenderPass(std::move(name))
		{
			{
				m_vtxLayout.Append(VertexLayout::Position2D);

				// Define the vertex input layout.
				std::vector<D3D12_INPUT_ELEMENT_DESC> vec = m_vtxLayout.GetD3DLayout();
				D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

				for (size_t i = 0; i < vec.size(); ++i) {
					inputElementDescs[i] = vec[i];
				}

				UINT num32BitConstants[2] = { (sizeof(XMMATRIX) / 4) * 3 , 2 };

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

				PipelineDescription pipelineDesc{};
				pipelineDesc.numConstants = 2;
				pipelineDesc.num32BitConstants = num32BitConstants;
				pipelineDesc.numStaticSamplers = 1;
				pipelineDesc.staticSamplers = samplers;
				pipelineDesc.backFaceCulling = true;
				pipelineDesc.numElements = vec.size();
				pipelineDesc.inputElementDescs = inputElementDescs;
				pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader,  L"BlurOutline_VS.hlsl" };
				pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"BlurOutline_PS.hlsl" };
				pipelineDesc.blending = true;
				pipelineDesc.depthStencilMode = Mode::Mask;

				m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
				m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
			}

			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.GetResourcePtr(2));
		}

		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			m_renderTarget = std::dynamic_pointer_cast<RenderTarget>(gfx.GetResourcePtr(gfx.GetCurrentBackBufferIndex()));

			ID3D12Resource* blurTargetBuffer = gfx.GetResource(5).GetBuffer();
			gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			RenderPass::Execute(gfx);

			gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

	private:
		VertexLayout m_vtxLayout;
	};
}