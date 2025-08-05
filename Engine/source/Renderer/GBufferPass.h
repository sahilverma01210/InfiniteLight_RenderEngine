#pragma once
#include "RenderPass.h"
#include "CameraContainer.h"
#include "Model.h"

namespace Renderer
{
	class GBufferPass : public RenderPass
	{
		struct CurrentInstance
		{
			UINT index;
		};

	public:
		GBufferPass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name, CameraContainer& cameraContainer, std::vector<std::shared_ptr<Model>>& models)
			:
			RenderPass(renderGraph, gfx, std::move(name)),
			m_cameraContainer(cameraContainer),
			m_models(models)
		{
			m_renderTargets.resize(3);
			m_renderTargets[0] = std::make_shared<RenderTarget>(m_gfx, m_gfx.GetWidth(), m_gfx.GetHeight(), DXGI_FORMAT_R32G32B32A32_FLOAT);
			m_renderTargets[1] = std::make_shared<RenderTarget>(m_gfx, m_gfx.GetWidth(), m_gfx.GetHeight(), DXGI_FORMAT_R32G32B32A32_FLOAT);
			m_renderTargets[2] = std::make_shared<RenderTarget>(m_gfx, m_gfx.GetWidth(), m_gfx.GetHeight(), DXGI_FORMAT_R32G32B32A32_FLOAT);
			RenderGraph::m_frameResourceHandles["Diffuse"] = m_gfx.LoadResource(m_renderTargets[0], D3D12Resource::ViewType::SRV);
			RenderGraph::m_frameResourceHandles["Normal"] = m_gfx.LoadResource(m_renderTargets[1], D3D12Resource::ViewType::SRV);
			RenderGraph::m_frameResourceHandles["MetallicRough"] = m_gfx.LoadResource(m_renderTargets[2], D3D12Resource::ViewType::SRV);
			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(m_gfx.GetResourcePtr(RenderGraph::m_frameResourceHandles["Depth_Stencil"]));

			CreatePSO();
		}
		void CreatePSO()
		{
			DXGI_FORMAT* renderTargetFormats = new DXGI_FORMAT[m_renderTargets.size()];

			for (size_t i = 0; i < m_renderTargets.size(); i++) {
				renderTargetFormats[i] = m_renderTargets[i]->GetFormat();
			}

			UINT num32BitConstants[2] = { 11, 1 };

			PipelineDescription pipelineDesc{};
			pipelineDesc.numRenderTargets = m_renderTargets.size();
			pipelineDesc.renderTargetFormats = renderTargetFormats;
			pipelineDesc.numConstants = 2;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.numStaticSamplers = 2; // One extra Sampler for Shadow Texture.
			pipelineDesc.backFaceCulling = true;
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"GBuffer_VS.hlsl" };
			pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"GBuffer_PS.hlsl" };

			m_rootSignature = std::move(std::make_unique<RootSignature>(m_gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(m_gfx, pipelineDesc));
		}
		void Execute() noexcept(!IS_DEBUG) override
		{
			m_cameraContainer.UpdateCamera();

			m_gfx.ClearResource(RenderGraph::m_frameResourceHandles["Diffuse"]);
			m_gfx.ClearResource(RenderGraph::m_frameResourceHandles["Normal"]);
			m_gfx.ClearResource(RenderGraph::m_frameResourceHandles["MetallicRough"]);

			m_depthStencil->Clear();

			m_rootSignature->Bind();
			m_pipelineStateObject->Bind();

			m_gfx.SetRenderTargets(m_renderTargets, m_depthStencil);
			m_gfx.Set32BitRootConstants(0, 11, &RenderGraph::m_frameData);
			m_gfx.SetPrimitiveTopology(m_pipelineStateObject->GetTopologyType());

			for (auto& model : m_models)
			{
				for (auto& mesh : model->GetMeshPointers())
				{
					if (mesh->GetRenderEffects()[GetName()])
					{
						m_gfx.Set32BitRootConstants(1, 1, &m_currentInstance.index);
						m_currentInstance.index++;

						auto& drawData = mesh->GetDrawData();
						Draw(drawData);
					}
				}
				m_currentInstance.index = 0; // Reset instance index for the next model
			}
		}

	private:
		CurrentInstance m_currentInstance{};
		CameraContainer& m_cameraContainer;
		std::vector<std::shared_ptr<Model>>& m_models;
	};
}