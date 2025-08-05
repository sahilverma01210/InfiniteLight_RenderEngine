#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class BlurPass : public RenderPass
	{
		struct BlurResourceHandles
		{
			int blurTargetIdx;
			int renderTargetIdx;
			int kernelConstIdx;
		};

		struct alignas(16) Coefficient
		{
			float value;
		};

		__declspec(align(256u)) struct Kernel // Store Object of this Type in Raw Pointer because of alignment issues.
		{
			alignas(16) UINT numTaps;
			Coefficient coefficients[15];
		};

	public:
		BlurPass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name)
			:
			RenderPass(renderGraph, gfx, std::move(name), RenderPassType::Compute)
		{
			m_kernel = new Kernel();

			SetKernelGauss(m_radius, m_sigma);

			m_blurResourceHandles.blurTargetIdx = RenderGraph::m_frameResourceHandles["Object_Flat"];
			m_blurResourceHandles.renderTargetIdx = m_gfx.LoadResource(std::make_shared<MeshTexture>(m_gfx, "NULL_TEX"));
			m_blurResourceHandles.kernelConstIdx = m_gfx.LoadResource(std::make_shared<D3D12Buffer>(m_gfx, m_kernel, sizeof(*m_kernel)));

			CreatePSO();
		}
		~BlurPass()
		{
			delete m_kernel;
		}
		void SetKernelBox(int radius) noexcept(!IS_DEBUG)
		{
			assert(radius <= m_maxRadius);
			const int nTaps = radius * 2 + 1;
			m_kernel->numTaps = nTaps;
			const float c = 1.0f / nTaps;
			for (int i = 0; i < nTaps; i++)
			{
				m_kernel->coefficients[i].value = c;
			}
		}
		void SetKernelGauss(int radius, float sigma) noexcept(!IS_DEBUG)
		{
			assert(radius <= m_maxRadius);
			const int nTaps = radius * 2 + 1;
			m_kernel->numTaps = nTaps;
			float sum = 0.0f;
			for (int i = 0; i < nTaps; i++)
			{
				const auto x = float(i - radius);
				const auto g = gauss(x, sigma);
				sum += g;
				m_kernel->coefficients[i].value = g;
			}
			for (int i = 0; i < nTaps; i++)
			{
				m_kernel->coefficients[i].value = (float)m_kernel->coefficients[i].value / sum;
			}
		}
		void CreatePSO()
		{
			UINT num32BitConstants[2] = { 11, 3 };

			PipelineDescription pipelineDesc{};
			pipelineDesc.type = PipelineType::Compute;
			pipelineDesc.numConstants = 2;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.numStaticSamplers = 1;
			pipelineDesc.computeShader = D3D12Shader{ ShaderType::ComputeShader,  L"Blur_CS.hlsl" };

			m_rootSignature = std::move(std::make_unique<RootSignature>(m_gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(m_gfx, pipelineDesc));
		}
		void Execute() noexcept(!IS_DEBUG) override
		{
			ID3D12Resource* blurTargetBuffer = m_gfx.GetResource(m_blurResourceHandles.blurTargetIdx).GetBuffer();
			ID3D12Resource* renderTargetBuffer = m_gfx.GetResourcePtr(m_blurResourceHandles.renderTargetIdx)->GetBuffer();

			m_gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			m_gfx.TransitionResource(renderTargetBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			m_rootSignature->Bind();
			m_pipelineStateObject->Bind();

			m_gfx.Set32BitRootConstants(0, 11, &RenderGraph::m_frameData, PipelineType::Compute);
			m_gfx.Set32BitRootConstants(1, 3, &m_blurResourceHandles, PipelineType::Compute);

			m_gfx.Dispatch(DivideAndRoundUp(m_gfx.GetWidth(), 16), DivideAndRoundUp(m_gfx.GetHeight(), 16), 1);

			m_gfx.TransitionResource(renderTargetBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
			m_gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);

			m_gfx.CopyResource(blurTargetBuffer, renderTargetBuffer);
			
			m_gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_gfx.TransitionResource(renderTargetBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}

	private:
		static constexpr int m_maxRadius = 7;
		int m_radius = 5;
		float m_sigma = 8.0f;
		BlurResourceHandles m_blurResourceHandles{};
		Kernel* m_kernel;
	};
}