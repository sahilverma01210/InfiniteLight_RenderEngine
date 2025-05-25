#pragma once
#include "RenderPass.h"
#include "CameraContainer.h"

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

		__declspec(align(256u)) struct LightResourceHandles
		{
			ResourceHandle renderTargetIdx;
			ResourceHandle frameConstIdx;
			ResourceHandle lightConstIdx;
			ResourceHandle diffTexIdx;
			ResourceHandle normTexIdx;
			ResourceHandle metallicRoughTexIdx;
			ResourceHandle depthTexIdx;
		};

		__declspec(align(256u)) struct FrameCBuffer
		{
			XMFLOAT2 renderResolution;
		};

	public:
		LightingPass(D3D12RHI& gfx, std::string name, CameraContainer& cameraContainer)
			:
			RenderPass(std::move(name), RenderPassType::Compute),
			m_cameraContainer(cameraContainer)
		{
			m_frameCBuffer.renderResolution = XMFLOAT2(static_cast<float>(gfx.GetWidth()), static_cast<float>(gfx.GetHeight()));

			m_lightResourceHandles.renderTargetIdx = gfx.LoadResource(std::make_shared<MeshTextureBuffer>(gfx, "NULL_TEX"), ResourceType::ReadWriteTexture);
			m_lightResourceHandles.frameConstIdx = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_frameCBuffer), &m_frameCBuffer), ResourceType::Constant);
			m_lightResourceHandles.lightConstIdx = RenderGraph::m_lightDataHandles[0];
			m_lightResourceHandles.diffTexIdx = RenderGraph::m_frameResourceHandles["Diffuse"];
			m_lightResourceHandles.normTexIdx = RenderGraph::m_frameResourceHandles["Normal"];
			m_lightResourceHandles.metallicRoughTexIdx = RenderGraph::m_frameResourceHandles["MetallicRough"];
			m_lightResourceHandles.depthTexIdx = RenderGraph::m_frameResourceHandles["Depth_Stencil"];

			lightResourceHandlesIdx = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_lightResourceHandles), &m_lightResourceHandles), ResourceType::Constant);

			CreatePSO(gfx);
		}
		void CreatePSO(D3D12RHI& gfx)
		{
			UINT num32BitConstants[2] = { 1, 1 };

			PipelineDescription pipelineDesc{};
			pipelineDesc.type = PipelineType::Compute;
			pipelineDesc.numConstants = 2;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.numStaticSamplers = 2;
			pipelineDesc.computeShader = D3D12Shader{ ShaderType::ComputeShader,  L"Lighting_CS.hlsl" };

			m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			ID3D12Resource* renderTargetBuffer = gfx.GetResourcePtr(m_lightResourceHandles.renderTargetIdx)->GetBuffer();
			ID3D12Resource* diffuseTexBuffer = gfx.GetResourcePtr(m_lightResourceHandles.diffTexIdx)->GetBuffer();
			ID3D12Resource* normalTexBuffer = gfx.GetResourcePtr(m_lightResourceHandles.normTexIdx)->GetBuffer();
			ID3D12Resource* metallicRoughTexBuffer = gfx.GetResourcePtr(m_lightResourceHandles.metallicRoughTexIdx)->GetBuffer();
			ID3D12Resource* depthTexBuffer = gfx.GetResourcePtr(m_lightResourceHandles.depthTexIdx)->GetBuffer();

			gfx.TransitionResource(renderTargetBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			gfx.TransitionResource(diffuseTexBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			gfx.TransitionResource(normalTexBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			gfx.TransitionResource(metallicRoughTexBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			gfx.TransitionResource(depthTexBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);

			for (size_t i = 0; i < 6; i++)
			{
				gfx.TransitionResource(gfx.GetResourcePtr(RenderGraph::m_frameResourceHandles["Shadow_Depth"] + i)->GetBuffer(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			}

			gfx.SetGPUResources();

			m_rootSignature->Bind(gfx);
			m_pipelineStateObject->Bind(gfx);

			gfx.Set32BitRootConstants(0, 1, &RenderPass::m_cameraDataHandle, PipelineType::Compute);
			gfx.Set32BitRootConstants(1, 1, &lightResourceHandlesIdx, PipelineType::Compute);

			RenderPass::Execute(gfx);

			gfx.TransitionResource(diffuseTexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);
			gfx.TransitionResource(normalTexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);
			gfx.TransitionResource(metallicRoughTexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);
			gfx.TransitionResource(depthTexBuffer, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			for (size_t i = 0; i < 6; i++)
			{
				gfx.TransitionResource(gfx.GetResourcePtr(RenderGraph::m_frameResourceHandles["Shadow_Depth"] + i)->GetBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			}

			ID3D12Resource* finalTargetBuffer = gfx.GetResourcePtr(gfx.GetCurrentBackBufferIndex())->GetBuffer();

			gfx.TransitionResource(renderTargetBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
			gfx.TransitionResource(finalTargetBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);

			gfx.CopyResource(finalTargetBuffer, renderTargetBuffer);

			gfx.TransitionResource(renderTargetBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			gfx.TransitionResource(finalTargetBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

	private:
		CameraContainer& m_cameraContainer;
		LightResourceHandles m_lightResourceHandles{};
		FrameCBuffer m_frameCBuffer{};
		ResourceHandle lightResourceHandlesIdx = -1;
	};
}