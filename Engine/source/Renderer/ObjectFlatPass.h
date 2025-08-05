#pragma once
#include "RenderPass.h"
#include "Model.h"

namespace Renderer
{
	class ObjectFlatPass : public RenderPass
	{
	public:
		ObjectFlatPass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name, std::vector<std::shared_ptr<Model>>& models)
			:
			RenderPass(renderGraph, gfx, std::move(name)),
			m_models(models)
		{
			m_renderTargets.resize(1);
			m_renderTargets[0] = std::make_shared<RenderTarget>(m_gfx, m_gfx.GetWidth(), m_gfx.GetHeight());
			RenderGraph::m_frameResourceHandles["Object_Flat"] = m_gfx.LoadResource(m_renderTargets[0], D3D12Resource::ViewType::SRV);
			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(m_gfx.GetResourcePtr(RenderGraph::m_frameResourceHandles["Depth_Stencil"]));

			CreatePSO();
		}
		void CreatePSO()
		{
			DXGI_FORMAT* renderTargetFormats = new DXGI_FORMAT[m_renderTargets.size()];

			for (size_t i = 0; i < m_renderTargets.size(); i++) {
				renderTargetFormats[i] = m_renderTargets[i]->GetFormat();
			}

			UINT num32BitConstants[3] = { 11, sizeof(XMMATRIX) / 4 , 3 };

			PipelineDescription pipelineDesc{};
			pipelineDesc.numRenderTargets = m_renderTargets.size();
			pipelineDesc.renderTargetFormats = renderTargetFormats;
			pipelineDesc.numConstants = 3;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.backFaceCulling = true;
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Flat_VS.hlsl" };
			pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Flat_PS.hlsl" };
			pipelineDesc.depthStencilMode = Mode::Write;

			m_rootSignature = std::move(std::make_unique<RootSignature>(m_gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(m_gfx, pipelineDesc));
		}
		void Execute() noexcept(!IS_DEBUG) override
		{
			m_gfx.ClearResource(RenderGraph::m_frameResourceHandles["Object_Flat"]);

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
						auto& meshMat = mesh->GetTransforms().meshMat;						
						m_gfx.Set32BitRootConstants(1, sizeof(meshMat) / 4, &meshMat);

						auto color = Vector3{ 1.0f,0.4f,0.4f };
						m_gfx.Set32BitRootConstants(2, 3, &color);

						Draw(mesh->GetDrawData());
					}
				}
			}
		}

	private:
		std::vector<std::shared_ptr<Model>>& m_models;
	};
}