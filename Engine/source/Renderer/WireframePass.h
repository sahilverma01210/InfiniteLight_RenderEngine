#pragma once
#include "RenderPass.h"
#include "CameraProjection.h"

namespace Renderer
{
	class WireframePass : public RenderPass
	{
	public:
		WireframePass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name, CameraContainer& cameraContainer)
			:
			RenderPass(renderGraph, gfx, std::move(name)),
			m_cameraContainer(cameraContainer)
		{
			for (int i = 0; i < m_cameraContainer.GetNumCameras(); ++i) m_cameraProjections.push_back(std::make_shared<CameraProjection>(m_gfx, 1.0f, 9.0f / 16.0f , 0.5f, 400.0f ));

			m_renderTargets.resize(1);
			m_renderTargets[0] = m_gfx.GetResourcePtr(m_gfx.GetCurrentBackBufferIndex());
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
			pipelineDesc.topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Flat_VS.hlsl" };
			pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Flat_PS.hlsl" };
			pipelineDesc.depthStencilMode = Mode::DepthAlways;

			m_rootSignature = std::move(std::make_unique<RootSignature>(m_gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(m_gfx, pipelineDesc));
		}
		void Execute() noexcept(!IS_DEBUG) override
		{
			int index = 0;

			for (auto& camera : m_cameraContainer.GetCameras())
			{
				auto& cameraProj = *m_cameraProjections[index];
				cameraProj.Toggle(camera->GetState().enableCameraProjection && index != m_cameraContainer.GetActiveCameraIndex());

				if (cameraProj.IsEnabled())
				{					
					cameraProj.Update(camera->GetTransform().position, camera->GetTransform().rotation);
					cameraProj.SetVertices(camera->GetProjection().width, camera->GetProjection().height, camera->GetProjection().nearZ, camera->GetProjection().farZ);
				}

				index++;
			}

			m_renderTargets[0] = m_gfx.GetResourcePtr(m_gfx.GetCurrentBackBufferIndex());

			m_rootSignature->Bind();
			m_pipelineStateObject->Bind();

			m_gfx.SetRenderTargets(m_renderTargets, m_depthStencil);
			m_gfx.Set32BitRootConstants(0, 11, &RenderGraph::m_frameData);
			m_gfx.SetPrimitiveTopology(m_pipelineStateObject->GetTopologyType());

			for (auto& cameraProjection : m_cameraProjections)
			{
				if (!cameraProjection->IsEnabled()) continue;

				auto& meshMat = cameraProjection->GetTransforms().meshMat;
				m_gfx.Set32BitRootConstants(1, sizeof(meshMat) / 4, &meshMat);

				auto color = Vector3{ 0.6f,0.2f,0.2f };
				m_gfx.Set32BitRootConstants(2, 3, &color);

				auto& drawData = cameraProjection->GetDrawData();
				Draw(drawData);
			}
		}

	private:
		CameraContainer& m_cameraContainer;
		std::vector<std::shared_ptr<CameraProjection>> m_cameraProjections;
	};
}