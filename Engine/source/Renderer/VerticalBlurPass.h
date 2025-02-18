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

				PipelineDescription pipelineDesc{};
				pipelineDesc.numConstantBufferViews = 2;
				pipelineDesc.numShaderResourceViews = 1;
				pipelineDesc.numSamplers = 1;
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

			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.m_textureManager.GetTexturePtr(3));
		}

		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			m_renderTarget = std::dynamic_pointer_cast<RenderTarget>(gfx.m_textureManager.GetTexturePtr(gfx.GetCurrentBackBufferIndex() + 1));

			ID3D12Resource* blurTargetBuffer = gfx.m_textureManager.GetTexture(6).GetBuffer();
			gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			RenderPass::Execute(gfx);

			gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

	private:
		VertexLayout m_vtxLayout;
	};
}