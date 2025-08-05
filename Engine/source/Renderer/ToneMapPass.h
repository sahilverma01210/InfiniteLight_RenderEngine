#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class ToneMapPass : public RenderPass
	{
		struct ToneMapConstants
		{
			int inputTexIdx;
			int outputTexIdx;
		};
	public:
		ToneMapPass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name)
			:
			RenderPass(renderGraph, gfx, std::move(name), RenderPassType::Compute)
		{
			m_toneMappingConstants.inputTexIdx = RenderGraph::m_frameResourceHandles["PT_Output"];
			m_toneMappingConstants.outputTexIdx = m_gfx.LoadResource(std::make_shared<MeshTexture>(gfx, "NULL_TEX"));

			ID3D12Resource* outputBuffer = m_gfx.GetResourcePtr(m_toneMappingConstants.outputTexIdx)->GetBuffer();
			m_gfx.TransitionResource(outputBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			CreatePSO();
		}
		void CreatePSO()
		{
			UINT num32BitConstants[2] = { 11, 2 };

			PipelineDescription pipelineDesc{};
			pipelineDesc.type = PipelineType::Compute;
			pipelineDesc.numConstants = 2;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.numStaticSamplers = 1;
			pipelineDesc.computeShader = D3D12Shader{ ShaderType::ComputeShader,  L"Tonemap_CS.hlsl" };

			m_rootSignature = std::move(std::make_unique<RootSignature>(m_gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(m_gfx, pipelineDesc));
		}
		void Execute() noexcept(!IS_DEBUG) override
		{
			m_rootSignature->Bind();
			m_pipelineStateObject->Bind();

			m_gfx.Set32BitRootConstants(0, 11, &RenderGraph::m_frameData, PipelineType::Compute);
			m_gfx.Set32BitRootConstants(1, 2, &m_toneMappingConstants, PipelineType::Compute);

			m_gfx.Dispatch(DivideAndRoundUp(m_gfx.GetWidth(), 16), DivideAndRoundUp(m_gfx.GetHeight(), 16), 1);

			ID3D12Resource* outputBuffer = m_gfx.GetResourcePtr(m_toneMappingConstants.outputTexIdx)->GetBuffer();
			ID3D12Resource* finalTargetBuffer = m_gfx.GetResourcePtr(m_gfx.GetCurrentBackBufferIndex())->GetBuffer();

			m_gfx.TransitionResource(outputBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
			m_gfx.TransitionResource(finalTargetBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);

			m_gfx.CopyResource(finalTargetBuffer, outputBuffer);

			m_gfx.TransitionResource(outputBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			m_gfx.TransitionResource(finalTargetBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

	private:
		ToneMapConstants m_toneMappingConstants{};
	};
}