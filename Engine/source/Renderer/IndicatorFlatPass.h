#pragma once
#include "RenderPass.h"
#include "LightContainer.h"
#include "CameraContainer.h"
#include "CameraIndicator.h"

namespace Renderer
{
	class IndicatorFlatPass : public RenderPass
	{
	public:
		IndicatorFlatPass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name, CameraContainer& cameraContainer, LightContainer& lightContainer)
			:
			RenderPass(renderGraph, std::move(name)),
			m_cameraContainer(cameraContainer),
			m_lightContainer(lightContainer)
		{
			for (int i = 0; i < m_lightContainer.GetNumLights(); ++i) m_pointLightindicators.push_back(std::make_shared<PointLightIndicator>(gfx, dynamic_cast<PointLight&>(m_lightContainer.GetControlledLight()).GetRadius()));
			for (int i = 0; i < m_cameraContainer.GetNumCameras(); ++i) m_cameraIndicators.push_back(std::make_shared<CameraIndicator>(gfx));

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

			UINT num32BitConstants[3] = { 5, 2 * sizeof(XMMATRIX) / 4 , 1 };

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
			int index = 0;

			for (auto& camera : m_cameraContainer.GetCameras())
			{
				auto& cameraIndicator = *m_cameraIndicators[index];
				cameraIndicator.Toggle(camera->GetState().enableCameraIndicator && index != m_cameraContainer.GetActiveCameraIndex());

				if (cameraIndicator.IsEnabled())
				{
					cameraIndicator.Update(camera->GetTransform().position, camera->GetTransform().rotation);
				}

				index++;
			}

			index = 0;

			for (auto& light : m_lightContainer.GetLights())
			{
				auto& pointLightindicator = *m_pointLightindicators[index];
				pointLightindicator.Update(light->GetLightData().pos);

				index++;
			}			

			m_cameraContainer.UpdateCamera(gfx);

			m_renderTargets[0] = gfx.GetResourcePtr(gfx.GetCurrentBackBufferIndex());

			m_rootSignature->Bind(gfx);
			m_pipelineStateObject->Bind(gfx);

			gfx.SetRenderTargets(m_renderTargets, m_depthStencil);
			gfx.Set32BitRootConstants(0, 5, &RenderGraph::m_frameData);
			gfx.SetPrimitiveTopology(m_pipelineStateObject->GetTopologyType());

			for(auto& cameraIndicator : m_cameraIndicators)
			{
				if (!cameraIndicator->IsEnabled()) continue;

				auto& transforms = cameraIndicator->GetTransforms();
				auto materialHandle = cameraIndicator->GetMaterialIdx();
				gfx.Set32BitRootConstants(1, sizeof(transforms) / 4, &transforms);
				gfx.Set32BitRootConstants(2, 1, &materialHandle);

				Draw(gfx, cameraIndicator->GetDrawData());
			}

			for (auto& pointLightindicator : m_pointLightindicators)
			{
				auto& transforms = pointLightindicator->GetTransforms();
				auto materialHandle = pointLightindicator->GetMaterialIdx();
				gfx.Set32BitRootConstants(1, sizeof(transforms) / 4, &transforms);
				gfx.Set32BitRootConstants(2, 1, &materialHandle);

				Draw(gfx, pointLightindicator->GetDrawData());
			}
		}

	private:
		VertexLayout m_vtxLayout;
		LightContainer& m_lightContainer;
		CameraContainer& m_cameraContainer;
		std::vector<std::shared_ptr<PointLightIndicator>> m_pointLightindicators;
		std::vector<std::shared_ptr<CameraIndicator>> m_cameraIndicators;
	};
}