#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class ObjectOutlinePass : public RenderPass
	{
		__declspec(align(256u)) struct InputData
		{
			ResourceHandle inputFrameIdx;
		};

	public:
		ObjectOutlinePass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name)
			:
			RenderPass(renderGraph, std::move(name))
		{
			IndexedTriangleList indexedList = Plane::Make();

			m_inputData.inputFrameIdx = RenderGraph::m_frameResourceHandles["Object_Flat"];
			m_materialIdx = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &m_inputData, sizeof(m_inputData)));

			m_drawData.numIndices = indexedList.indices.size();
			m_drawData.vertexSizeInBytes = UINT(indexedList.vertices.SizeBytes());
			m_drawData.indexSizeInBytes = m_drawData.numIndices * sizeof(indexedList.indices[0]);
			m_drawData.vertexStrideInBytes = (UINT)indexedList.vertices.GetLayout().Size();

			m_drawData.vertexBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, indexedList.vertices.GetData(), m_drawData.vertexSizeInBytes));
			m_drawData.indexBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, indexedList.indices.data(), m_drawData.indexSizeInBytes));

			m_renderTargets.resize(1);
			m_renderTargets[0] = gfx.GetResourcePtr(gfx.GetCurrentBackBufferIndex());
			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.GetResourcePtr(RenderGraph::m_frameResourceHandles["Depth_Stencil"]));

			CreatePSO(gfx);
		}
		void CreatePSO(D3D12RHI& gfx)
		{
			m_vtxLayout.Append(VertexLayout::Position2D);

			// Define the vertex input layout.
			std::vector<D3D12_INPUT_ELEMENT_DESC> vec = m_vtxLayout.GetD3DLayout();
			D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

			for (size_t i = 0; i < vec.size(); ++i) {
				inputElementDescs[i] = vec[i];
			}

			DXGI_FORMAT* renderTargetFormats = new DXGI_FORMAT[m_renderTargets.size()];

			for (size_t i = 0; i < m_renderTargets.size(); i++) {
				renderTargetFormats[i] = m_renderTargets[i]->GetFormat();
			}

			UINT num32BitConstants[3] = { 5, 2 * sizeof(XMMATRIX) / 4 , 1 };

			PipelineDescription pipelineDesc{};
			pipelineDesc.numRenderTargets = m_renderTargets.size();
			pipelineDesc.renderTargetFormats = renderTargetFormats;
			pipelineDesc.numConstants = 3;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.numStaticSamplers = 1;
			pipelineDesc.backFaceCulling = true;
			pipelineDesc.numElements = vec.size();
			pipelineDesc.inputElementDescs = inputElementDescs;
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader,  L"ObjectOutline_VS.hlsl" };
			pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"ObjectOutline_PS.hlsl" };
			pipelineDesc.blending = true;
			pipelineDesc.depthStencilMode = Mode::Mask;

			m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			m_transforms.meshMat = XMMatrixIdentity();
			m_transforms.meshInvMat = XMMatrixInverse(nullptr, m_transforms.meshMat);

			m_renderTargets[0] = gfx.GetResourcePtr(gfx.GetCurrentBackBufferIndex());

			ID3D12Resource* outlineBuffer = gfx.GetResource(RenderGraph::m_frameResourceHandles["Object_Flat"]).GetBuffer();
			gfx.TransitionResource(outlineBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			m_rootSignature->Bind(gfx);
			m_pipelineStateObject->Bind(gfx);

			gfx.SetRenderTargets(m_renderTargets, m_depthStencil);
			gfx.Set32BitRootConstants(0, 5, &RenderGraph::m_frameData);
			gfx.SetPrimitiveTopology(m_pipelineStateObject->GetTopologyType());

			gfx.Set32BitRootConstants(1, sizeof(m_transforms) / 4, &m_transforms);
			gfx.Set32BitRootConstants(2, 1, &m_materialIdx);

			Draw(gfx, m_drawData);

			gfx.TransitionResource(outlineBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

	private:
		VertexLayout m_vtxLayout;
		ILMesh::DrawData m_drawData{};
		ResourceHandle m_materialIdx;
		mutable ILMesh::Transforms m_transforms{};
		InputData m_inputData{};
	};
}