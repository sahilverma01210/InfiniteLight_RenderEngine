#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class ObjectOutlinePass : public RenderPass
	{
		struct InputData
		{
			ResourceHandle inputFrameIdx;
		};

	public:
		ObjectOutlinePass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name)
			:
			RenderPass(renderGraph, gfx, std::move(name))
		{
			IndexedTriangleList indexedList = Plane::Make();

			m_inputData.inputFrameIdx = RenderGraph::m_frameResourceHandles["Object_Flat"];

			m_drawData.vertices = indexedList.vertices;
			m_drawData.indices = indexedList.indices;
			m_drawData.vertexSizeInBytes = m_drawData.vertices.size() * sizeof(m_drawData.vertices[0]);
			m_drawData.indexSizeInBytes = m_drawData.indices.size() * sizeof(m_drawData.indices[0]);
			m_drawData.vertexStrideInBytes = sizeof(VertexStruct);

			m_drawData.vertexBuffer = std::move(std::make_shared<D3D12Buffer>(m_gfx, m_drawData.vertices.data(), m_drawData.vertexSizeInBytes));
			m_drawData.indexBuffer = std::move(std::make_shared<D3D12Buffer>(m_gfx, m_drawData.indices.data(), m_drawData.indexSizeInBytes));

			m_renderTargets.resize(1);
			m_renderTargets[0] = m_gfx.GetResourcePtr(m_gfx.GetCurrentBackBufferIndex());
			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(m_gfx.GetResourcePtr(RenderGraph::m_frameResourceHandles["Depth_Stencil"]));

			CreatePSO();
		}
		void CreatePSO()
		{
			DXGI_FORMAT* renderTargetFormats = new DXGI_FORMAT[m_renderTargets.size()];

			for (size_t i = 0; i < m_renderTargets.size(); i++) {
				renderTargetFormats[i] = m_renderTargets[i]->GetFormat();
			}

			UINT num32BitConstants[2] = { 11, 1 };

			PipelineDescription pipelineDesc{};
			pipelineDesc.numRenderTargets = m_renderTargets.size();
			pipelineDesc.renderTargetFormats = renderTargetFormats;
			pipelineDesc.numConstants = 2;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.numStaticSamplers = 1;
			pipelineDesc.backFaceCulling = true;
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader,  L"Outline_VS.hlsl" };
			pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Outline_PS.hlsl" };
			pipelineDesc.blending = true;
			pipelineDesc.depthStencilMode = Mode::Mask;

			m_rootSignature = std::move(std::make_unique<RootSignature>(m_gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(m_gfx, pipelineDesc));
		}
		void Execute() noexcept(!IS_DEBUG) override
		{
			m_transforms.meshMat = XMMatrixIdentity();
			m_transforms.meshInvMat = XMMatrixInverse(nullptr, m_transforms.meshMat);

			m_renderTargets[0] = m_gfx.GetResourcePtr(m_gfx.GetCurrentBackBufferIndex());

			ID3D12Resource* outlineBuffer = m_gfx.GetResource(RenderGraph::m_frameResourceHandles["Object_Flat"]).GetBuffer();
			m_gfx.TransitionResource(outlineBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			m_rootSignature->Bind();
			m_pipelineStateObject->Bind();

			m_gfx.SetRenderTargets(m_renderTargets, m_depthStencil);
			m_gfx.Set32BitRootConstants(0, 11, &RenderGraph::m_frameData);
			m_gfx.SetPrimitiveTopology(m_pipelineStateObject->GetTopologyType());

			m_gfx.Set32BitRootConstants(1, 1, &m_inputData);

			Draw(m_drawData);

			m_gfx.TransitionResource(outlineBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

	private:
		ILMesh::DrawData m_drawData{};
		mutable ILMesh::Transforms m_transforms{};
		InputData m_inputData{};
	};
}