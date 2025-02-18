#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class BlurOutlineDrawingPass : public RenderPass
	{
	public:
		BlurOutlineDrawingPass(D3D12RHI& gfx, std::string name, unsigned int fullWidth, unsigned int fullHeight)
			:
			RenderPass(std::move(name))
		{
			{
				m_vtxLayout.Append(VertexLayout::Position3D);

				// Define the vertex input layout.
				std::vector<D3D12_INPUT_ELEMENT_DESC> vec = m_vtxLayout.GetD3DLayout();
				D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

				for (size_t i = 0; i < vec.size(); ++i) {
					inputElementDescs[i] = vec[i];
				}

				PipelineDescription drawPipelineDesc{};
				drawPipelineDesc.numConstants = 1;
				drawPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
				drawPipelineDesc.numConstantBufferViews = 1;
				drawPipelineDesc.backFaceCulling = true;
				drawPipelineDesc.numElements = vec.size();
				drawPipelineDesc.inputElementDescs = inputElementDescs;
				drawPipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Flat_VS.hlsl" };
				drawPipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Flat_PS.hlsl" };
				drawPipelineDesc.depthUsage = DepthUsage::None;

				m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, drawPipelineDesc));
				m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, drawPipelineDesc));
			}

			m_renderTarget = std::make_shared<RenderTarget>(gfx, fullWidth, fullHeight);
			gfx.m_textureManager.LoadTexture(m_renderTarget);
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			m_renderTarget->Clear(gfx);
			RenderPass::Execute(gfx);
		}

	private:
		VertexLayout m_vtxLayout;
	};
}