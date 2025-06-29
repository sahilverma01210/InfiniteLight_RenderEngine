#pragma once
#include "RenderPass.h"
#include "CameraContainer.h"
#include "Model.h"

namespace Renderer
{
	class GBufferPass : public RenderPass
	{
	public:
		GBufferPass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name, CameraContainer& cameraContainer, std::vector<std::shared_ptr<Model>>& models)
			:
			RenderPass(renderGraph, std::move(name)),
			m_cameraContainer(cameraContainer),
			m_models(models)
		{
			m_renderTargets.resize(3);
			m_renderTargets[0] = std::make_shared<RenderTarget>(gfx, gfx.GetWidth(), gfx.GetHeight(), DXGI_FORMAT_R32G32B32A32_FLOAT);
			m_renderTargets[1] = std::make_shared<RenderTarget>(gfx, gfx.GetWidth(), gfx.GetHeight(), DXGI_FORMAT_R32G32B32A32_FLOAT);
			m_renderTargets[2] = std::make_shared<RenderTarget>(gfx, gfx.GetWidth(), gfx.GetHeight(), DXGI_FORMAT_R32G32B32A32_FLOAT);
			RenderGraph::m_frameResourceHandles["Diffuse"] = gfx.LoadResource(m_renderTargets[0], D3D12Resource::ViewType::SRV);
			RenderGraph::m_frameResourceHandles["Normal"] = gfx.LoadResource(m_renderTargets[1], D3D12Resource::ViewType::SRV);
			RenderGraph::m_frameResourceHandles["MetallicRough"] = gfx.LoadResource(m_renderTargets[2], D3D12Resource::ViewType::SRV);
			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.GetResourcePtr(RenderGraph::m_frameResourceHandles["Depth_Stencil"]));

			CreatePSO(gfx);

			//m_renderGraph.AppendPass(std::make_unique<GBufferPass>(*this));
		}
		void CreatePSO(D3D12RHI& gfx)
		{
			m_vtxLayout.Append(VertexLayout::Position3D);
			m_vtxLayout.Append(VertexLayout::Normal);
			m_vtxLayout.Append(VertexLayout::Texture2D);
			m_vtxLayout.Append(VertexLayout::Tangent);
			m_vtxLayout.Append(VertexLayout::Bitangent);

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

			UINT num32BitConstants[3] = { 5, 2 * sizeof(XMMATRIX) / 4 , 1 };

			PipelineDescription pipelineDesc{};
			pipelineDesc.numRenderTargets = m_renderTargets.size();
			pipelineDesc.renderTargetFormats = renderTargetFormats;
			pipelineDesc.numConstants = 3;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.numStaticSamplers = 2; // One extra Sampler for Shadow Texture.
			pipelineDesc.backFaceCulling = true;
			pipelineDesc.numElements = vec.size();
			pipelineDesc.inputElementDescs = inputElementDescs;
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"GBuffer_VS.hlsl" };
			pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"GBuffer_PS.hlsl" };

			m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			m_cameraContainer.UpdateCamera(gfx);

			gfx.ClearResource(RenderGraph::m_frameResourceHandles["Diffuse"]);
			gfx.ClearResource(RenderGraph::m_frameResourceHandles["Normal"]);
			gfx.ClearResource(RenderGraph::m_frameResourceHandles["MetallicRough"]);

			m_depthStencil->Clear(gfx);

			m_rootSignature->Bind(gfx);
			m_pipelineStateObject->Bind(gfx);

			gfx.SetRenderTargets(m_renderTargets, m_depthStencil);
			gfx.Set32BitRootConstants(0, 5, &RenderGraph::m_frameData);
			gfx.SetPrimitiveTopology(m_pipelineStateObject->GetTopologyType());

			for (auto& model : m_models)
			{
				for (auto& mesh : model->GetMeshes())
				{
					if (mesh->GetRenderEffects()[GetName()])
					{
						auto& transforms = mesh->GetTransforms();
						auto materialHandle = mesh->GetMaterialIdx();
						gfx.Set32BitRootConstants(1, sizeof(transforms) / 4, &transforms);
						gfx.Set32BitRootConstants(2, 1, &materialHandle);

						Draw(gfx, mesh->GetDrawData());
					}
				}
			}
		}

	private:
		VertexLayout m_vtxLayout;
		CameraContainer& m_cameraContainer;
		std::vector<std::shared_ptr<Model>>& m_models;
	};
}