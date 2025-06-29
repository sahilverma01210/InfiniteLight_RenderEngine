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
			RenderPass(renderGraph, std::move(name)),
			m_cameraContainer(cameraContainer)
		{
			for (int i = 0; i < m_cameraContainer.GetNumCameras(); ++i) m_cameraProjections.push_back(std::make_shared<CameraProjection>(gfx, 1.0f, 9.0f / 16.0f , 0.5f, 400.0f ));

			m_renderTargets.resize(1);
			m_renderTargets[0] = gfx.GetResourcePtr(gfx.GetCurrentBackBufferIndex());
			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.GetResourcePtr(RenderGraph::m_frameResourceHandles["Depth_Stencil"]));

			CreatePSO(gfx);

			//m_renderGraph.AppendPass(std::make_unique<WireframePass>(*this));
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
			pipelineDesc.numElements = vec.size();
			pipelineDesc.inputElementDescs = inputElementDescs;
			pipelineDesc.topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Flat_VS.hlsl" };
			pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Flat_PS.hlsl" };
			pipelineDesc.depthStencilMode = Mode::DepthAlways;

			m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			int index = 0;

			for (auto& camera : m_cameraContainer.GetCameras())
			{
				auto& cameraProj = *m_cameraProjections[index];
				cameraProj.Toggle(camera->GetState().enableCameraProjection && index != m_cameraContainer.GetActiveCameraIndex());

				if (cameraProj.IsEnabled())
				{					
					cameraProj.Update(camera->GetTransform().position, camera->GetTransform().rotation);
					cameraProj.SetVertices(gfx, camera->GetProjection().width, camera->GetProjection().height, camera->GetProjection().nearZ, camera->GetProjection().farZ);
				}

				index++;
			}

			m_renderTargets[0] = gfx.GetResourcePtr(gfx.GetCurrentBackBufferIndex());

			m_rootSignature->Bind(gfx);
			m_pipelineStateObject->Bind(gfx);

			gfx.SetRenderTargets(m_renderTargets, m_depthStencil);
			gfx.Set32BitRootConstants(0, 5, &RenderGraph::m_frameData);
			gfx.SetPrimitiveTopology(m_pipelineStateObject->GetTopologyType());

			for (auto& cameraProjection : m_cameraProjections)
			{
				if (!cameraProjection->IsEnabled()) continue;

				auto& transforms = cameraProjection->GetTransforms();
				auto materialHandle = cameraProjection->GetMaterialIdx();
				gfx.Set32BitRootConstants(1, sizeof(transforms) / 4, &transforms);
				gfx.Set32BitRootConstants(2, 1, &materialHandle);

				Draw(gfx, cameraProjection->GetDrawData());
			}
		}

	private:
		VertexLayout m_vtxLayout;
		CameraContainer& m_cameraContainer;
		std::vector<std::shared_ptr<CameraProjection>> m_cameraProjections;
	};
}