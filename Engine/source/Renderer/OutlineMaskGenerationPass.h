#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class OutlineMaskGenerationPass : public RenderPass
	{
	public:
		OutlineMaskGenerationPass(D3D12RHI& gfx, std::string name)
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

				UINT num32BitConstants[2] = { (sizeof(XMMATRIX) / 4) * 3 , 2 };

				PipelineDescription maskPipelineDesc{};
				maskPipelineDesc.numConstants = 2;
				maskPipelineDesc.num32BitConstants = num32BitConstants;
				maskPipelineDesc.depthStencilMode = Mode::Write;
				maskPipelineDesc.numElements = vec.size();
				maskPipelineDesc.inputElementDescs = inputElementDescs;
				maskPipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Flat_VS.hlsl" };

				m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, maskPipelineDesc));
				m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, maskPipelineDesc));
			}

			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.GetResourcePtr(2));
			m_depthOnlyPass = true;
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			RenderPass::Execute(gfx);
		}

	private:
		VertexLayout m_vtxLayout;
	};
}