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
			RenderPass(renderGraph, gfx, std::move(name)),
			m_lightContainer(lightContainer),
			m_cameraContainer(cameraContainer),
			m_models(models)
		{
			for (auto& light : m_lightContainer.GetLights())
			{
				for (size_t i = 0; i < 6; i++)
				{
					ResourceHandle handle = m_gfx.LoadResource(std::make_shared<DepthStencil>(m_gfx, 2048, 2048, DepthUsage::ShadowDepth));
					if (!i) light->SetShadowMapHandle(handle);
				}
			}			

			m_depthStencil = std::make_shared<DepthStencil>(m_gfx, 2048, 2048, DepthUsage::ShadowDepth);

			CreatePSO();
		}
		void CreatePSO()
		{
			UINT num32BitConstants[3] = { 11, sizeof(XMMATRIX) / 4, 2};

			PipelineDescription pipelineDesc{};
			pipelineDesc.numConstants = 3;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.shadowMapping = true;
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Shadow_VS.hlsl" };
			pipelineDesc.depthUsage = DepthUsage::ShadowDepth;

			m_rootSignature = std::move(std::make_unique<RootSignature>(m_gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(m_gfx, pipelineDesc));
		}
		void Execute() noexcept(!IS_DEBUG) override
		{
			UINT initWidth = m_gfx.GetWidth();
			UINT initHeight = m_gfx.GetHeight();

			m_gfx.ResizeScreenSpace(m_size, m_size);

			UINT lightIdx = 0;

			for (auto& light : m_lightContainer.GetLights())
			{
				for (size_t faceIdx = 0; faceIdx < 6; faceIdx++)
				{
					m_depthStencil->Clear();

					m_rootSignature->Bind();
					m_pipelineStateObject->Bind();

					m_shadowIndices.currentLightIndex = lightIdx;
					m_shadowIndices.currentfaceIndex = faceIdx;

					m_gfx.SetRenderTargets(m_renderTargets, m_depthStencil);
					m_gfx.Set32BitRootConstants(0, 11, &RenderGraph::m_frameData);
					m_gfx.SetPrimitiveTopology(m_pipelineStateObject->GetTopologyType());

					for (auto& model : m_models)
					{
						for (auto& mesh : model->GetMeshPointers())
						{
							if (mesh->GetRenderEffects()[GetName()])
							{
								auto& meshMat = mesh->GetTransforms().meshMat;
								m_gfx.Set32BitRootConstants(1, sizeof(meshMat) / 4, &meshMat);

								m_gfx.Set32BitRootConstants(2, 2, &m_shadowIndices);

								auto& drawData = mesh->GetDrawData();
								Draw(drawData);
							}
						}
					}

					ID3D12Resource* depthStencilBuffer = m_depthStencil->GetBuffer();
					ID3D12Resource* depthTargetBuffer = m_gfx.GetResourcePtr(light->GetShadowMapHandle() + faceIdx)->GetBuffer();

					m_gfx.TransitionResource(depthStencilBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_SOURCE);
					m_gfx.TransitionResource(depthTargetBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_DEST);
					m_gfx.CopyResource(depthTargetBuffer, depthStencilBuffer);
					m_gfx.TransitionResource(depthStencilBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
					m_gfx.TransitionResource(depthTargetBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_DEPTH_WRITE);
				}

				lightIdx++;
			}
			
			m_gfx.ResizeScreenSpace(initWidth, initHeight);
		}

	private:
		ShadowIndices m_shadowIndices{};
		static constexpr UINT m_size = 2048;
		LightContainer& m_lightContainer;
		CameraContainer& m_cameraContainer;
		std::vector<std::shared_ptr<Model>>& m_models;
	};
}