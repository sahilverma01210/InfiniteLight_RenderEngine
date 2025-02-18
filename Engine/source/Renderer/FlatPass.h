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

				PipelineDescription phongPipelineDesc{};
				phongPipelineDesc.numConstants = 1;
				phongPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
				phongPipelineDesc.numConstantBufferViews = 1;
				phongPipelineDesc.numElements = vec.size();
				phongPipelineDesc.inputElementDescs = inputElementDescs;
				phongPipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Flat_VS.hlsl" };
				phongPipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Flat_PS.hlsl" };

				m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, phongPipelineDesc));
				m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, phongPipelineDesc));
			}

			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.m_textureManager.GetTexturePtr(3));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			m_cameraContainer.GetActiveCamera().Update();

			m_renderTarget = std::dynamic_pointer_cast<RenderTarget>(gfx.m_textureManager.GetTexturePtr(gfx.GetCurrentBackBufferIndex() + 1));
			RenderPass::Execute(gfx);
		}

	private:
		VertexLayout m_vtxLayout;
		CameraContainer& m_cameraContainer;
	};
}