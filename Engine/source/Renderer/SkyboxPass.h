#pragma once  
#include "RenderPass.h"  
#include "Camera.h"  
#include "Shapes.h"  

namespace Renderer  
{  
	class SkyboxPass : public RenderPass  
	{  
	public:  
		SkyboxPass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name)  
			:  
			RenderPass(renderGraph, std::move(name))  
		{  
			// Initialize the skybox and draw data
			{
				IndexedTriangleList indexedList = Cube::Make();
				indexedList.Transform(XMMatrixScaling(3.0f, 3.0f, 3.0f));

				m_drawData.numIndices = indexedList.indices.size();
				m_drawData.vertexSizeInBytes = UINT(indexedList.vertices.SizeBytes());
				m_drawData.indexSizeInBytes = m_drawData.numIndices * sizeof(indexedList.indices[0]);
				m_drawData.vertexStrideInBytes = (UINT)indexedList.vertices.GetLayout().Size();

				m_drawData.vertexBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, indexedList.vertices.GetData(), m_drawData.vertexSizeInBytes));
				m_drawData.indexBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, indexedList.indices.data(), m_drawData.indexSizeInBytes));
			}

			m_renderTargets.resize(1);  
			m_renderTargets[0] = gfx.GetResourcePtr(gfx.GetCurrentBackBufferIndex());  
			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.GetResourcePtr(RenderGraph::m_frameResourceHandles["Depth_Stencil"]));  

			CreatePSO(gfx);  

			//m_renderGraph.AppendPass(std::make_unique<SkyboxPass>(*this));
		}  
		void CreatePSO(D3D12RHI& gfx)  
		{  
			m_vtxLayout.Append(VertexLayout::Position3D);  

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

			UINT num32BitConstants[2] = { 5, 2 * sizeof(XMMATRIX) / 4 };  

			PipelineDescription pipelineDesc{};  
			pipelineDesc.numRenderTargets = m_renderTargets.size();  
			pipelineDesc.renderTargetFormats = renderTargetFormats;  
			pipelineDesc.numConstants = 2;  
			pipelineDesc.num32BitConstants = num32BitConstants;  
			pipelineDesc.numStaticSamplers = 1;  
			pipelineDesc.depthStencilMode = Mode::DepthFirst;  
			pipelineDesc.numElements = vec.size();  
			pipelineDesc.inputElementDescs = inputElementDescs;  
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader,  L"Skybox_VS.hlsl" };  
			pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Skybox_PS.hlsl" };  

			m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));  
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));  
		}  
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override  
		{
			m_transforms.meshMat = XMMatrixIdentity();
			m_transforms.meshInvMat = XMMatrixInverse(nullptr, m_transforms.meshMat);

			m_renderTargets[0] = gfx.GetResourcePtr(gfx.GetCurrentBackBufferIndex());  

			m_rootSignature->Bind(gfx);  
			m_pipelineStateObject->Bind(gfx);  

			gfx.SetRenderTargets(m_renderTargets, m_depthStencil);  
			gfx.Set32BitRootConstants(0, 5, &RenderGraph::m_frameData);  
			gfx.SetPrimitiveTopology(m_pipelineStateObject->GetTopologyType());  

			gfx.Set32BitRootConstants(1, sizeof(m_transforms) / 4, &m_transforms);

			Draw(gfx, m_drawData);
		}  

	private:  
		VertexLayout m_vtxLayout;
		ILMesh::DrawData m_drawData{};
		mutable ILMesh::Transforms m_transforms{};
	};  
}