#pragma once
#include "RenderPass.h"
#include "RenderGraph.h"
#include "CameraContainer.h"
#include "AccelerationStructure.h"

namespace Renderer
{
	class PathTracingPass : public RenderPass
	{
		struct PathTracingPassData
		{
			Int32   bounce_count;
			Int32   accumulated_frames;
			ResourceHandle accumulation;
			ResourceHandle output;
		};
	public:
		PathTracingPass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name, CameraContainer& cameraContainer)
			:
			RenderPass(renderGraph, gfx, std::move(name), RenderPassType::Compute),
			m_cameraContainer(cameraContainer)
		{
			TextureDesc accDesc{}, outDesc{};
			accDesc.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			outDesc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;

			m_pathTracingPassData.accumulation = m_gfx.LoadResource(std::make_shared<MeshTexture>(gfx, "NULL_TEX", accDesc));
			m_pathTracingPassData.output = RenderGraph::m_frameResourceHandles["PT_Output"] = m_gfx.LoadResource(std::make_shared<MeshTexture>(gfx, "NULL_TEX", outDesc));
			m_pathTracingPassData.bounce_count = 3;
			m_pathTracingPassData.accumulated_frames = 1;

			ID3D12Resource* outputBuffer = m_gfx.GetResourcePtr(m_pathTracingPassData.output)->GetBuffer();
			m_gfx.TransitionResource(outputBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			ID3D12Resource* accumulationBuffer = m_gfx.GetResourcePtr(m_pathTracingPassData.accumulation)->GetBuffer();
			m_gfx.TransitionResource(accumulationBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			CreateStateObject();
		}
		void CreateStateObject()
		{
			UINT num32BitConstants[2] = { 11, 4 };

			PipelineDescription pipelineDesc{};
			pipelineDesc.type = PipelineType::Compute;
			pipelineDesc.numConstants = 2;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.numStaticSamplers = 2;
			pipelineDesc.computeShader = D3D12Shader{ ShaderType::LibraryShader,  L"PathTracer.hlsl" };
			pipelineDesc.stateObjectType = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

			m_rootSignature = std::move(std::make_unique<RootSignature>(m_gfx, pipelineDesc));
			m_stateObject = std::move(std::make_unique<D3D12StateObject>(m_gfx, pipelineDesc));
		}
		void Execute() noexcept(!IS_DEBUG) override
		{
			if (m_cameraContainer.UpdateCamera()) m_pathTracingPassData.accumulated_frames = 1;

			m_rootSignature->Bind();

			m_gfx.Set32BitRootConstants(0, 11, &RenderGraph::m_frameData, PipelineType::Compute);
			m_gfx.Set32BitRootConstants(1, 4, &m_pathTracingPassData, PipelineType::Compute);

			DispatchRays("PT_RayGen");

			++m_pathTracingPassData.accumulated_frames;
		}

	private:
		CameraContainer& m_cameraContainer;
		PathTracingPassData m_pathTracingPassData{};
	};
}