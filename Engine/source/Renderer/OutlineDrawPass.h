#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class OutlineDrawPass : public RenderPass
	{
	public:
		OutlineDrawPass(D3D12RHI& gfx, std::string name)
			:
			RenderPass(std::move(name))
		{
			CreatePSO(gfx);

			m_renderTargets.resize(1);
			m_renderTargets[0] = std::make_shared<RenderTarget>(gfx, gfx.GetWidth(), gfx.GetHeight());
			RenderGraph::m_renderTargetHandles["Outline_Draw"] = gfx.LoadResource(m_renderTargets[0], ResourceType::Texture);
			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.GetResourcePtr(RenderGraph::m_depthStencilHandle));
		}
		void CreatePSO(D3D12RHI& gfx)
		{
			m_vtxLayout.Append(VertexLayout::Position3D);

			// Define the vertex input layout.
			std::vector<D3D12_INPUT_ELEMENT_DESC> vec = m_vtxLayout.GetD3DLayout();
			D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

			for (size_t i = 0; i < vec.size(); ++i) {
				inputElementDescs[i] = vec[i];
			}

			DXGI_FORMAT* renderTargetFormats = new DXGI_FORMAT[1];
			renderTargetFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

			UINT num32BitConstants[2] = { (sizeof(XMMATRIX) / 4) * 3 , 2 };

			PipelineDescription pipelineDesc{};
			pipelineDesc.numRenderTargets = 1;
			pipelineDesc.renderTargetFormats = renderTargetFormats;
			pipelineDesc.numConstants = 2;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.depthStencilMode = Mode::Write;
			pipelineDesc.backFaceCulling = true;
			pipelineDesc.numElements = vec.size();
			pipelineDesc.inputElementDescs = inputElementDescs;
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Outline_VS.hlsl" };
			pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Outline_PS.hlsl" };

			m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			gfx.ClearResource(RenderGraph::m_renderTargetHandles["Outline_Draw"], ResourceType::RenderTarget);
			RenderPass::Execute(gfx);
		}

	private:
		VertexLayout m_vtxLayout;
	};
}