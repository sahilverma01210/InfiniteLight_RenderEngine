#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class BlurPass : public RenderPass
	{
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
		BlurPass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name)
			:
			RenderPass(renderGraph, std::move(name), RenderPassType::Compute)
		{
			m_frameCBuffer.renderResolution = XMFLOAT2(static_cast<float>(gfx.GetWidth()), static_cast<float>(gfx.GetHeight()));
			SetKernelGauss(m_radius, m_sigma);

			m_blurResourceHandles.blurTargetIdx = RenderGraph::m_frameResourceHandles["Object_Flat"];
			m_blurResourceHandles.renderTargetIdx = gfx.LoadResource(std::make_shared<MeshTexture>(gfx, "NULL_TEX"));
			m_blurResourceHandles.frameConstIdx = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &m_frameCBuffer, sizeof(m_frameCBuffer)));
			m_blurResourceHandles.kernelConstIdx = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &m_kernel, sizeof(m_kernel)));

			blurResourceHandlesIdx = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &m_blurResourceHandles, sizeof(m_blurResourceHandles)));

			CreatePSO(gfx);

			//m_renderGraph.AppendPass(std::make_unique<BlurPass>(*this));
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
			UINT num32BitConstants[2] = { 5, 1 };

			PipelineDescription pipelineDesc{};
			pipelineDesc.type = PipelineType::Compute;
			pipelineDesc.numConstants = 2;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.numStaticSamplers = 1;
			pipelineDesc.computeShader = D3D12Shader{ ShaderType::ComputeShader,  L"Blur_CS.hlsl" };

			m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			ID3D12Resource* blurTargetBuffer = gfx.GetResource(m_blurResourceHandles.blurTargetIdx).GetBuffer();
			ID3D12Resource* renderTargetBuffer = gfx.GetResourcePtr(m_blurResourceHandles.renderTargetIdx)->GetBuffer();

			gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			gfx.TransitionResource(renderTargetBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			m_rootSignature->Bind(gfx);
			m_pipelineStateObject->Bind(gfx);

			gfx.Set32BitRootConstants(0, 5, &RenderGraph::m_frameData, PipelineType::Compute);
			gfx.Set32BitRootConstants(1, 1, &blurResourceHandlesIdx, PipelineType::Compute);

			gfx.Dispatch(DivideAndRoundUp(gfx.GetWidth(), 16), DivideAndRoundUp(gfx.GetHeight(), 16), 1);

			gfx.TransitionResource(renderTargetBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
			gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);

			gfx.CopyResource(blurTargetBuffer, renderTargetBuffer);
			
			gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
			gfx.TransitionResource(renderTargetBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}

	private:
		static constexpr int m_maxRadius = 7;
		int m_radius = 5;
		float m_sigma = 8.0f;
		BlurResourceHandles m_blurResourceHandles{};
		FrameCBuffer m_frameCBuffer{};
		Kernel m_kernel = {};
		ResourceHandle blurResourceHandlesIdx = -1;
	};
}