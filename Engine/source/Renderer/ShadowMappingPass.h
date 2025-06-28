#pragma once
#include "RenderPass.h"
#include "RenderMath.h"
#include "LightContainer.h"
#include "CameraContainer.h"
#include "Model.h"

namespace Renderer
{
	class ShadowMappingPass : public RenderPass
	{
		struct ShadowIndices
		{
			UINT currentLightIndex;
			UINT currentfaceIndex;
		};

	public:
		ShadowMappingPass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name, CameraContainer& cameraContainer, LightContainer& lightContainer, std::vector<std::shared_ptr<Model>>& models)
			:
			RenderPass(renderGraph, std::move(name)),
			m_lightContainer(lightContainer),
			m_cameraContainer(cameraContainer),
			m_models(models)
		{
			for (auto& light : m_lightContainer.GetLights())
			{
				for (size_t i = 0; i < 6; i++)
				{
					ResourceHandle handle = gfx.LoadResource(std::make_shared<DepthStencil>(gfx, 2048, 2048, DepthUsage::ShadowDepth));
					if (!i) light->SetShadowMapHandle(handle);
				}
			}			

			m_depthStencil = std::make_shared<DepthStencil>(gfx, 2048, 2048, DepthUsage::ShadowDepth);

			CreatePSO(gfx);

			//m_renderGraph.AppendPass(std::make_unique<ShadowMappingPass>(*this));
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

			UINT num32BitConstants[4] = { 5, 2 * sizeof(XMMATRIX) / 4, 1, 2};

			PipelineDescription pipelineDesc{};
			pipelineDesc.numConstants = 4;
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

			UINT lightIdx = 0;

			for (auto& light : m_lightContainer.GetLights())
			{
				for (size_t faceIdx = 0; faceIdx < 6; faceIdx++)
				{
					m_depthStencil->Clear(gfx);

					m_rootSignature->Bind(gfx);
					m_pipelineStateObject->Bind(gfx);

					m_shadowIndices.currentLightIndex = lightIdx;
					m_shadowIndices.currentfaceIndex = faceIdx;

					gfx.Set32BitRootConstants(3, 2, &m_shadowIndices);

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

					ID3D12Resource* depthStencilBuffer = m_depthStencil->GetBuffer();
					ID3D12Resource* depthTargetBuffer = gfx.GetResourcePtr(light->GetShadowMapHandle() + faceIdx)->GetBuffer();

					gfx.TransitionResource(depthStencilBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_SOURCE);
					gfx.TransitionResource(depthTargetBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_DEST);
					gfx.CopyResource(depthTargetBuffer, depthStencilBuffer);
					gfx.TransitionResource(depthStencilBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
					gfx.TransitionResource(depthTargetBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_DEPTH_WRITE);
				}

				lightIdx++;
			}
			
			gfx.ResizeScreenSpace(initWidth, initHeight);
		}

	private:
		ShadowIndices m_shadowIndices{};
		VertexLayout m_vtxLayout;
		static constexpr UINT m_size = 2048;
		LightContainer& m_lightContainer;
		CameraContainer& m_cameraContainer;
		std::vector<std::shared_ptr<Model>>& m_models;
	};
}