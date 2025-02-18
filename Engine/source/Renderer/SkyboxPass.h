#pragma once
#include "RenderPass.h"
#include "Camera.h"
#include "Shapes.h"

namespace Renderer
{
	class SkyboxPass : public RenderPass
	{
	public:
		SkyboxPass(D3D12RHI& gfx, std::string name)
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

				PipelineDescription skyboxPipelineDesc{};

				skyboxPipelineDesc.numConstants = 1;
				skyboxPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
				skyboxPipelineDesc.numShaderResourceViews = 1;
				skyboxPipelineDesc.numSamplers = 1;
				skyboxPipelineDesc.depthStencilMode = Mode::DepthFirst;
				skyboxPipelineDesc.numElements = vec.size();
				skyboxPipelineDesc.inputElementDescs = inputElementDescs;
				skyboxPipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader,  L"Skybox_VS.hlsl" };
				skyboxPipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Skybox_PS.hlsl" };

				m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, skyboxPipelineDesc));
				m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, skyboxPipelineDesc));
			}

			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.m_textureManager.GetTexturePtr(3));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			m_renderTarget = std::dynamic_pointer_cast<RenderTarget>(gfx.m_textureManager.GetTexturePtr(gfx.GetCurrentBackBufferIndex() + 1));
			RenderPass::Execute(gfx);
		}

	private:
		VertexLayout m_vtxLayout;
	};
}