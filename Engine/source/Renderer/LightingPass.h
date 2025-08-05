#pragma once
#include "RenderPass.h"
#include "LightContainer.h"

namespace Renderer
{
	class LightingPass : public RenderPass
	{
		struct Transforms
		{
			Matrix meshMat;
			Matrix viewMat;
			Matrix projectionMat;
		};

		struct LightResourceHandles
		{
			int32_t numLights;
			ResourceHandle renderTargetIdx;
			ResourceHandle diffTexIdx;
			ResourceHandle normTexIdx;
			ResourceHandle metallicRoughTexIdx;
			ResourceHandle depthTexIdx;
		};

	public:
		LightingPass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name, LightContainer& lightContainer)
			:
			RenderPass(renderGraph, gfx, std::move(name), RenderPassType::Compute),
			m_lightContainer(lightContainer)
		{
			m_lightResourceHandles.numLights = lightContainer.GetLights().size();
			m_lightResourceHandles.renderTargetIdx = m_gfx.LoadResource(std::make_shared<MeshTexture>(m_gfx, "NULL_TEX"));
			m_lightResourceHandles.diffTexIdx = RenderGraph::m_frameResourceHandles["Diffuse"];
			m_lightResourceHandles.normTexIdx = RenderGraph::m_frameResourceHandles["Normal"];
			m_lightResourceHandles.metallicRoughTexIdx = RenderGraph::m_frameResourceHandles["MetallicRough"];
			m_lightResourceHandles.depthTexIdx = RenderGraph::m_frameResourceHandles["Depth_Stencil"];

			CreatePSO();
		}
		void CreatePSO()
		{
			UINT num32BitConstants[2] = { 11, 6 };

			PipelineDescription pipelineDesc{};
			pipelineDesc.type = PipelineType::Compute;
			pipelineDesc.numConstants = 2;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.numStaticSamplers = 2;
			pipelineDesc.computeShader = D3D12Shader{ ShaderType::ComputeShader,  L"Lighting_CS.hlsl" };

			m_rootSignature = std::move(std::make_unique<RootSignature>(m_gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(m_gfx, pipelineDesc));
		}
		void Execute() noexcept(!IS_DEBUG) override
		{
			ID3D12Resource* renderTargetBuffer = m_gfx.GetResourcePtr(m_lightResourceHandles.renderTargetIdx)->GetBuffer();
			ID3D12Resource* diffuseTexBuffer = m_gfx.GetResourcePtr(m_lightResourceHandles.diffTexIdx)->GetBuffer();
			ID3D12Resource* normalTexBuffer = m_gfx.GetResourcePtr(m_lightResourceHandles.normTexIdx)->GetBuffer();
			ID3D12Resource* metallicRoughTexBuffer = m_gfx.GetResourcePtr(m_lightResourceHandles.metallicRoughTexIdx)->GetBuffer();
			ID3D12Resource* depthTexBuffer = m_gfx.GetResourcePtr(m_lightResourceHandles.depthTexIdx)->GetBuffer();

			m_gfx.TransitionResource(renderTargetBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			m_gfx.TransitionResource(diffuseTexBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			m_gfx.TransitionResource(normalTexBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			m_gfx.TransitionResource(metallicRoughTexBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			m_gfx.TransitionResource(depthTexBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);

			for (auto& light : m_lightContainer.GetLights())
			{
				for (size_t i = 0; i < 6; i++)
				{
					m_gfx.TransitionResource(m_gfx.GetResourcePtr(light->GetShadowMapHandle() + i)->GetBuffer(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				}
			}

			m_rootSignature->Bind();
			m_pipelineStateObject->Bind();

			m_gfx.Set32BitRootConstants(0, 11, &RenderGraph::m_frameData, PipelineType::Compute);
			m_gfx.Set32BitRootConstants(1, 6, &m_lightResourceHandles, PipelineType::Compute);

			m_gfx.Dispatch(DivideAndRoundUp(m_gfx.GetWidth(), 16), DivideAndRoundUp(m_gfx.GetHeight(), 16), 1);

			m_gfx.TransitionResource(diffuseTexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_gfx.TransitionResource(normalTexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_gfx.TransitionResource(metallicRoughTexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_gfx.TransitionResource(depthTexBuffer, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			for (auto& light : m_lightContainer.GetLights())
			{
				for (size_t i = 0; i < 6; i++)
				{
					m_gfx.TransitionResource(m_gfx.GetResourcePtr(light->GetShadowMapHandle() + i)->GetBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
				}
			}

			ID3D12Resource* finalTargetBuffer = m_gfx.GetResourcePtr(m_gfx.GetCurrentBackBufferIndex())->GetBuffer();

			m_gfx.TransitionResource(renderTargetBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
			m_gfx.TransitionResource(finalTargetBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);

			m_gfx.CopyResource(finalTargetBuffer, renderTargetBuffer);

			m_gfx.TransitionResource(renderTargetBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			m_gfx.TransitionResource(finalTargetBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

	private:
		LightContainer& m_lightContainer;
		LightResourceHandles m_lightResourceHandles{};
	};
}