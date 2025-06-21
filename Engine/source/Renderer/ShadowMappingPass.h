#pragma once
#include "RenderPass.h"
#include "RenderMath.h"
#include "CameraContainer.h"

namespace Renderer
{
	class ShadowMappingPass : public RenderPass
	{
	public:
		ShadowMappingPass(D3D12RHI& gfx, std::string name, CameraContainer& cameraContainer)
			:
			RenderPass(std::move(name)),
			m_cameraContainer(cameraContainer)
		{
			for (size_t i = 0; i < 6; i++)
			{
				ResourceHandle handle = gfx.LoadResource(std::make_shared<DepthStencil>(gfx, 2048, 2048, DepthUsage::ShadowDepth));
				if (!i) RenderGraph::m_frameResourceHandles["Shadow_Depth"] = handle;
			}

			m_depthStencil = std::make_shared<DepthStencil>(gfx, 2048, 2048, DepthUsage::ShadowDepth);

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

			UINT num32BitConstants[5] = { 1, 2 * sizeof(XMMATRIX) / 4, 1, 1, sizeof(XMFLOAT3) / 4};

			PipelineDescription pipelineDesc{};
			pipelineDesc.numConstants = 5;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.shadowMapping = true;
			pipelineDesc.numElements = vec.size();
			pipelineDesc.inputElementDescs = inputElementDescs;
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Shadow_VS.hlsl" };
			pipelineDesc.depthUsage = DepthUsage::ShadowDepth;

			m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			UINT initWidth = gfx.GetWidth();
			UINT initHeight = gfx.GetHeight();

			gfx.ResizeScreenSpace(m_size, m_size);

			for (size_t i = 0; i < 6; i++)
			{
				//m_depthStencil = std::move(m_pDepthCube->GetDepthBuffer(i));
				m_depthStencil->Clear(gfx);

				m_rootSignature->Bind(gfx);
				m_pipelineStateObject->Bind(gfx);

				gfx.Set32BitRootConstants(3, 1, &i);
				gfx.Set32BitRootConstants(4, sizeof(RenderGraph::m_lightPosition) / 4, &RenderGraph::m_lightPosition);
				
				RenderPass::Execute(gfx);

				ID3D12Resource* depthStencilBuffer = m_depthStencil->GetBuffer();
				ID3D12Resource* depthTargetBuffer = gfx.GetResourcePtr(RenderGraph::m_frameResourceHandles["Shadow_Depth"] + i)->GetBuffer();

				gfx.TransitionResource(depthStencilBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_SOURCE);
				gfx.TransitionResource(depthTargetBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_DEST);
				gfx.CopyResource(depthTargetBuffer, depthStencilBuffer);
				gfx.TransitionResource(depthStencilBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
				gfx.TransitionResource(depthTargetBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			}
			gfx.ResizeScreenSpace(initWidth, initHeight);
		}

	private:
		VertexLayout m_vtxLayout;
		static constexpr UINT m_size = 2048;
		CameraContainer& m_cameraContainer;
	};
}