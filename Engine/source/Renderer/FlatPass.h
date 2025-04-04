#pragma once
#include "RenderPass.h"
#include "CameraContainer.h"

namespace Renderer
{
	class FlatPass : public RenderPass
	{
	public:
		FlatPass(D3D12RHI& gfx, std::string name, CameraContainer& cameraContainer)
			:
			RenderPass(std::move(name)),
			m_cameraContainer(cameraContainer)
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

				PipelineDescription phongPipelineDesc{};
				phongPipelineDesc.numConstants = 2;
				phongPipelineDesc.num32BitConstants = num32BitConstants;
				phongPipelineDesc.numElements = vec.size();
				phongPipelineDesc.inputElementDescs = inputElementDescs;
				phongPipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Flat_VS.hlsl" };
				phongPipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Flat_PS.hlsl" };

				m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, phongPipelineDesc));
				m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, phongPipelineDesc));
			}

			m_renderTargets.resize(1);
			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.GetResourcePtr(RenderGraph::m_depthStencilHandle));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			m_cameraContainer.GetActiveCamera().Update();

			m_renderTargets[0] = gfx.GetResourcePtr(gfx.GetCurrentBackBufferIndex());
			RenderPass::Execute(gfx);
		}

	private:
		VertexLayout m_vtxLayout;
		CameraContainer& m_cameraContainer;
	};
}