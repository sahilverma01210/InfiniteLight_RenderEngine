#pragma once
#include "RenderPass.h"
#include "CameraContainer.h"

namespace Renderer
{
	class LightingPass : public RenderPass
	{
		struct MaterialHandle
		{
			int lightResourceHandlesIdx;
		};

		__declspec(align(256u)) struct LightResourceHandles
		{
			int renderTargetIdx;
			int frameConstIdx;
			int lightConstIdx;
			int shadowTexIdx;
			int fragPosTexIdx;
			int diffTexIdx;
			int normTexIdx;
			int specTexIdx;
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
			m_lightResourceHandles.lightConstIdx = ILMaterial::m_lightHandle;
			m_lightResourceHandles.shadowTexIdx = RenderGraph::m_shadowDepth360Handle;
			m_lightResourceHandles.fragPosTexIdx = RenderGraph::m_renderTargetHandles["Position"];
			m_lightResourceHandles.diffTexIdx = RenderGraph::m_renderTargetHandles["Diffuse"];
			m_lightResourceHandles.normTexIdx = RenderGraph::m_renderTargetHandles["Normal"];
			m_lightResourceHandles.specTexIdx = RenderGraph::m_renderTargetHandles["Specular"];

			matHandle.lightResourceHandlesIdx = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_lightResourceHandles), &m_lightResourceHandles), ResourceType::Constant);

			CreatePSO(gfx);
		}
		void CreatePSO(D3D12RHI& gfx)
		{
			UINT num32BitConstants[2] = { sizeof(XMMATRIX) / 4, 1 };

			CD3DX12_STATIC_SAMPLER_DESC* staticSamplers = new CD3DX12_STATIC_SAMPLER_DESC[2];

			CD3DX12_STATIC_SAMPLER_DESC sampler1{ 0 };
			sampler1.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			sampler1.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler1.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler1.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
			sampler1.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			staticSamplers[0] = sampler1;
			// define static sampler 
			CD3DX12_STATIC_SAMPLER_DESC sampler2{ 1, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
			sampler2.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			sampler2.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			sampler2.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			sampler2.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
			sampler2.MipLODBias = 0.0f;
			sampler2.MinLOD = 0.0f;
			sampler2.MaxLOD = D3D12_FLOAT32_MAX;
			staticSamplers[1] = sampler2;

			PipelineDescription pipelineDesc{};
			pipelineDesc.type = PipelineType::Compute;
			pipelineDesc.numConstants = 2;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.numStaticSamplers = 2;
			pipelineDesc.staticSamplers = staticSamplers;
			pipelineDesc.computeShader = D3D12Shader{ ShaderType::ComputeShader,  L"Lighting_CS.hlsl" };

			m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			ID3D12Resource* renderTargetBuffer = gfx.GetResourcePtr(m_lightResourceHandles.renderTargetIdx)->GetBuffer();
			ID3D12Resource* positionTexBuffer = gfx.GetResourcePtr(m_lightResourceHandles.fragPosTexIdx)->GetBuffer();
			ID3D12Resource* diffuseTexBuffer = gfx.GetResourcePtr(m_lightResourceHandles.diffTexIdx)->GetBuffer();
			ID3D12Resource* normalTexBuffer = gfx.GetResourcePtr(m_lightResourceHandles.normTexIdx)->GetBuffer();
			ID3D12Resource* specularTexBuffer = gfx.GetResourcePtr(m_lightResourceHandles.specTexIdx)->GetBuffer();

			gfx.TransitionResource(renderTargetBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			gfx.TransitionResource(positionTexBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			gfx.TransitionResource(diffuseTexBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			gfx.TransitionResource(normalTexBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			gfx.TransitionResource(specularTexBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			gfx.SetGPUResources();

			m_rootSignature->Bind(gfx);
			m_pipelineStateObject->Bind(gfx);

			auto cameraCBuf = m_cameraContainer.GetActiveCamera().GetCameraProps();

			gfx.Set32BitRootConstants(0, sizeof(cameraCBuf) / 4, &cameraCBuf, PipelineType::Compute);
			gfx.Set32BitRootConstants(1, 1, &matHandle, PipelineType::Compute);

			RenderPass::Execute(gfx);

			gfx.TransitionResource(positionTexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);
			gfx.TransitionResource(diffuseTexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);
			gfx.TransitionResource(normalTexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);
			gfx.TransitionResource(specularTexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);

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
		MaterialHandle matHandle{};
	};
}