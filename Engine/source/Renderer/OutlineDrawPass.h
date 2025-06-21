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
			m_renderTargets.resize(1);
			m_renderTargets[0] = std::make_shared<RenderTarget>(gfx, gfx.GetWidth(), gfx.GetHeight());
			RenderGraph::m_frameResourceHandles["Outline_Draw"] = gfx.LoadResource(m_renderTargets[0], D3D12Resource::ViewType::SRV);
			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.GetResourcePtr(RenderGraph::m_frameResourceHandles["Depth_Stencil"]));

			CreatePSO(gfx);
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

			DXGI_FORMAT* renderTargetFormats = new DXGI_FORMAT[m_renderTargets.size()];

			for (size_t i = 0; i < m_renderTargets.size(); i++) {
				renderTargetFormats[i] = m_renderTargets[i]->GetFormat();
			}

			UINT num32BitConstants[3] = { 1, 2 * sizeof(XMMATRIX) / 4 , 1 };

			PipelineDescription pipelineDesc{};
			pipelineDesc.numRenderTargets = m_renderTargets.size();
			pipelineDesc.renderTargetFormats = renderTargetFormats;
			pipelineDesc.numConstants = 3;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.backFaceCulling = true;
			pipelineDesc.numElements = vec.size();
			pipelineDesc.inputElementDescs = inputElementDescs;
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Outline_VS.hlsl" };
			pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Outline_PS.hlsl" };
			pipelineDesc.depthStencilMode = Mode::Write;

			m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			gfx.ClearResource(RenderGraph::m_frameResourceHandles["Outline_Draw"]);

			m_rootSignature->Bind(gfx);
			m_pipelineStateObject->Bind(gfx);

			RenderPass::Execute(gfx);
		}

	private:
		VertexLayout m_vtxLayout;
	};
}