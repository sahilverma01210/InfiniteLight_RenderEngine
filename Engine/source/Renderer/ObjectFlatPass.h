#pragma once
#include "RenderPass.h"
#include "Model.h"

namespace Renderer
{
	class ObjectFlatPass : public RenderPass
	{
		__declspec(align(256u)) struct SolidMatHandles
		{
			ResourceHandle solidConstIdx;
		};

		__declspec(align(256u)) struct SolidCB
		{
			Vector3 materialColor;
		};

	public:
		ObjectFlatPass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name, std::vector<std::shared_ptr<Model>>& models)
			:
			RenderPass(renderGraph, std::move(name)),
			m_models(models)
		{
			m_solidMatHandles.solidConstIdx = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &data, sizeof(data)));
			m_materialHandle = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &m_solidMatHandles, sizeof(m_solidMatHandles)));

			m_renderTargets.resize(1);
			m_renderTargets[0] = std::make_shared<RenderTarget>(gfx, gfx.GetWidth(), gfx.GetHeight());
			RenderGraph::m_frameResourceHandles["Object_Flat"] = gfx.LoadResource(m_renderTargets[0], D3D12Resource::ViewType::SRV);
			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.GetResourcePtr(RenderGraph::m_frameResourceHandles["Depth_Stencil"]));

			CreatePSO(gfx);

			//m_renderGraph.AppendPass(std::make_unique<OutlineDrawPass>(*this));
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

			UINT num32BitConstants[3] = { 5, 2 * sizeof(XMMATRIX) / 4 , 1 };

			PipelineDescription pipelineDesc{};
			pipelineDesc.numRenderTargets = m_renderTargets.size();
			pipelineDesc.renderTargetFormats = renderTargetFormats;
			pipelineDesc.numConstants = 3;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.backFaceCulling = true;
			pipelineDesc.numElements = vec.size();
			pipelineDesc.inputElementDescs = inputElementDescs;
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Flat_VS.hlsl" };
			pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Flat_PS.hlsl" };
			pipelineDesc.depthStencilMode = Mode::Write;

			m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			gfx.ClearResource(RenderGraph::m_frameResourceHandles["Object_Flat"]);

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
						
						gfx.Set32BitRootConstants(1, sizeof(transforms) / 4, &transforms);
						gfx.Set32BitRootConstants(2, 1, &m_materialHandle);

						Draw(gfx, mesh->GetDrawData());
					}
				}
			}
		}

	private:
		ResourceHandle m_materialHandle;
		SolidCB data = { Vector3{ 1.0f,0.4f,0.4f } };
		VertexLayout m_vtxLayout;
		SolidMatHandles m_solidMatHandles{};
		std::vector<std::shared_ptr<Model>>& m_models;
	};
}