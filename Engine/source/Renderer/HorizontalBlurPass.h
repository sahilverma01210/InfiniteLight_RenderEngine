#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class HorizontalBlurPass : public RenderPass
	{
	public:
		HorizontalBlurPass(D3D12RHI& gfx, std::string name, unsigned int fullWidth, unsigned int fullHeight)
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

				PipelineDescription pipelineDesc{};
				pipelineDesc.numConstantBufferViews = 2;
				pipelineDesc.numShaderResourceViews = 1;
				pipelineDesc.numSamplers = 1;
				pipelineDesc.backFaceCulling = true;
				pipelineDesc.numElements = vec.size();
				pipelineDesc.inputElementDescs = inputElementDescs;
				pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"BlurOutline_VS.hlsl" };
				pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"BlurOutline_PS.hlsl" };

				m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
				m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
			}

			m_renderTarget = std::make_shared<RenderTarget>(gfx, fullWidth, fullHeight);
			gfx.m_textureManager.LoadTexture(m_renderTarget);
		}

		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			ID3D12Resource* blurTargetBuffer = gfx.m_textureManager.GetTexture(5).GetBuffer();
			gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			RenderPass::Execute(gfx);

			gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

	private:
		VertexLayout m_vtxLayout;
	};
}