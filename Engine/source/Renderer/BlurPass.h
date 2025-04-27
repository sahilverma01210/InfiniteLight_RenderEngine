#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class BlurPass : public RenderPass
	{
		struct MaterialHandle
		{
			int blurResourceHandlesIdx;
		};

		__declspec(align(256u)) struct BlurResourceHandles
		{
			int blurTargetIdx;
			int renderTargetIdx;
			int frameConstIdx;
			int kernelConstIdx;
		};

		__declspec(align(256u)) struct FrameCBuffer
		{
			XMFLOAT2 renderResolution;
		};

		struct alignas(16) Coefficient
		{
			float value;
		};

		__declspec(align(256u)) struct Kernel
		{
			alignas(16) UINT numTaps;
			Coefficient coefficients[15];
		};

	public:
		BlurPass(D3D12RHI& gfx, std::string name)
			:
			RenderPass(std::move(name), RenderPassType::Compute)
		{
			m_frameCBuffer.renderResolution = XMFLOAT2(static_cast<float>(gfx.GetWidth()), static_cast<float>(gfx.GetHeight()));
			SetKernelGauss(m_radius, m_sigma);

			m_blurResourceHandles.blurTargetIdx = RenderGraph::m_renderTargetHandles["Outline_Draw"];
			m_blurResourceHandles.renderTargetIdx = gfx.LoadResource(std::make_shared<MeshTextureBuffer>(gfx, "NULL_TEX"), ResourceType::ReadWriteTexture);
			m_blurResourceHandles.frameConstIdx = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_frameCBuffer), &m_frameCBuffer), ResourceType::Constant);
			m_blurResourceHandles.kernelConstIdx = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_kernel), &m_kernel), ResourceType::Constant);

			matHandle.blurResourceHandlesIdx = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_blurResourceHandles), &m_blurResourceHandles), ResourceType::Constant);

			CreatePSO(gfx);
		}
		void SetKernelBox(int radius) noexcept(!IS_DEBUG)
		{
			assert(radius <= m_maxRadius);
			const int nTaps = radius * 2 + 1;
			m_kernel.numTaps = nTaps;
			const float c = 1.0f / nTaps;
			for (int i = 0; i < nTaps; i++)
			{
				m_kernel.coefficients[i].value = c;
			}
		}
		void SetKernelGauss(int radius, float sigma) noexcept(!IS_DEBUG)
		{
			assert(radius <= m_maxRadius);
			const int nTaps = radius * 2 + 1;
			m_kernel.numTaps = nTaps;
			float sum = 0.0f;
			for (int i = 0; i < nTaps; i++)
			{
				const auto x = float(i - radius);
				const auto g = gauss(x, sigma);
				sum += g;
				m_kernel.coefficients[i].value = g;
			}
			for (int i = 0; i < nTaps; i++)
			{
				m_kernel.coefficients[i].value = (float)m_kernel.coefficients[i].value / sum;
			}
		}
		void CreatePSO(D3D12RHI& gfx)
		{
			UINT num32BitConstants[1] = { 1 };

			CD3DX12_STATIC_SAMPLER_DESC* samplers = new CD3DX12_STATIC_SAMPLER_DESC[1];
			// define static sampler 
			CD3DX12_STATIC_SAMPLER_DESC staticSampler{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
			staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			staticSampler.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
			staticSampler.MipLODBias = 0.0f;
			staticSampler.MinLOD = 0.0f;
			staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
			samplers[0] = staticSampler;

			PipelineDescription pipelineDesc{};
			pipelineDesc.type = PipelineType::Compute;
			pipelineDesc.numConstants = 1;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.numStaticSamplers = 1;
			pipelineDesc.staticSamplers = samplers;
			pipelineDesc.computeShader = D3D12Shader{ ShaderType::ComputeShader,  L"Blur_CS.hlsl" };

			m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			ID3D12Resource* blurTargetBuffer = gfx.GetResource(m_blurResourceHandles.blurTargetIdx).GetBuffer();
			ID3D12Resource* renderTargetBuffer = gfx.GetResourcePtr(m_blurResourceHandles.renderTargetIdx)->GetBuffer();

			//gfx.ClearResource(m_frameHandles.renderTargetHandle, ResourceType::ReadWriteTexture);
			gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			gfx.TransitionResource(renderTargetBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
						
			gfx.SetGPUResources();

			m_rootSignature->Bind(gfx);
			m_pipelineStateObject->Bind(gfx);

			gfx.Set32BitRootConstants(0, 1, &matHandle, PipelineType::Compute);

			RenderPass::Execute(gfx);

			//ID3D12Resource* finalTargetBuffer = gfx.GetResourcePtr(gfx.GetCurrentBackBufferIndex())->GetBuffer();

			gfx.TransitionResource(renderTargetBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
			gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);

			gfx.CopyResource(blurTargetBuffer, renderTargetBuffer);
			
			gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
			gfx.TransitionResource(renderTargetBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			//gfx.TransitionResource(finalTargetBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

	private:
		static constexpr int m_maxRadius = 7;
		int m_radius = 5;
		float m_sigma = 8.0f;
		BlurResourceHandles m_blurResourceHandles{};
		FrameCBuffer m_frameCBuffer{};
		Kernel m_kernel = {};
		MaterialHandle matHandle{};
	};
}