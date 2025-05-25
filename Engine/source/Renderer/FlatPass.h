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
			m_renderTargets.resize(1);
			m_renderTargets[0] = gfx.GetResourcePtr(gfx.GetCurrentBackBufferIndex());
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
			pipelineDesc.numElements = vec.size();
			pipelineDesc.inputElementDescs = inputElementDescs;
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Flat_VS.hlsl" };
			pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Flat_PS.hlsl" };

			m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			m_cameraContainer.UpdateCamera(gfx);

			m_renderTargets[0] = gfx.GetResourcePtr(gfx.GetCurrentBackBufferIndex());

			m_rootSignature->Bind(gfx);
			m_pipelineStateObject->Bind(gfx);

			RenderPass::Execute(gfx);
		}

	private:
		VertexLayout m_vtxLayout;
		CameraContainer& m_cameraContainer;
	};
}