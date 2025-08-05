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
			RenderPass(renderGraph, gfx, std::move(name)),
			m_cameraContainer(cameraContainer),
			m_lightContainer(lightContainer)
		{
			for (int i = 0; i < m_lightContainer.GetNumLights(); ++i) m_pointLightindicators.push_back(std::make_shared<PointLightIndicator>(m_gfx, dynamic_cast<PointLight&>(m_lightContainer.GetControlledLight()).GetRadius()));
			for (int i = 0; i < m_cameraContainer.GetNumCameras(); ++i) m_cameraIndicators.push_back(std::make_shared<CameraIndicator>(m_gfx));

			m_renderTargets.resize(1);
			m_renderTargets[0] = m_gfx.GetResourcePtr(m_gfx.GetCurrentBackBufferIndex());
			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(m_gfx.GetResourcePtr(RenderGraph::m_frameResourceHandles["Depth_Stencil"]));

			CreatePSO();
		}
		~IndicatorFlatPass()
		{
			m_pointLightindicators.clear();
			m_cameraIndicators.clear();
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
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Flat_VS.hlsl" };
			pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Flat_PS.hlsl" };

			m_rootSignature = std::move(std::make_unique<RootSignature>(m_gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(m_gfx, pipelineDesc));
		}
		void Execute() noexcept(!IS_DEBUG) override
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
				pointLightindicator.Update(light->GetLightData().position);

				index++;
			}			

			m_cameraContainer.UpdateCamera();

			m_renderTargets[0] = m_gfx.GetResourcePtr(m_gfx.GetCurrentBackBufferIndex());

			m_rootSignature->Bind();
			m_pipelineStateObject->Bind();

			m_gfx.SetRenderTargets(m_renderTargets, m_depthStencil);
			m_gfx.Set32BitRootConstants(0, 11, &RenderGraph::m_frameData);
			m_gfx.SetPrimitiveTopology(m_pipelineStateObject->GetTopologyType());

			for(auto& cameraIndicator : m_cameraIndicators)
			{
				if (!cameraIndicator->IsEnabled()) continue;

				auto& meshMat = cameraIndicator->GetTransforms().meshMat;
				m_gfx.Set32BitRootConstants(1, sizeof(meshMat) / 4, &meshMat);

				auto color = Vector3{ 0.0f,0.0f,1.0f };
				m_gfx.Set32BitRootConstants(2, 3, &color);

				auto& drawData = cameraIndicator->GetDrawData();
				Draw(drawData);
			}

			for (auto& pointLightindicator : m_pointLightindicators)
			{
				auto& meshMat = pointLightindicator->GetTransforms().meshMat;
				m_gfx.Set32BitRootConstants(1, sizeof(meshMat) / 4, &meshMat);

				auto color = Vector3{ 1.0f,1.0f,1.0f };
				m_gfx.Set32BitRootConstants(2, 3, &color);

				auto& drawData = pointLightindicator->GetDrawData();
				Draw(drawData);
			}
		}

	private:
		LightContainer& m_lightContainer;
		CameraContainer& m_cameraContainer;
		std::vector<std::shared_ptr<PointLightIndicator>> m_pointLightindicators;
		std::vector<std::shared_ptr<CameraIndicator>> m_cameraIndicators;
	};
}