#pragma once  
#include "RenderPass.h"  
#include "Camera.h"  
#include "Shapes.h"  

namespace Renderer  
{  
	class SkyboxPass : public RenderPass  
	{  
		struct Transforms
		{
			Matrix meshMat;
		};
	public:  
		SkyboxPass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name)  
			:  
			RenderPass(renderGraph, gfx, std::move(name))  
		{  
			// Initialize the skybox and draw data
			{
				IndexedTriangleList indexedList = Cube::Make();
				indexedList.Transform(XMMatrixScaling(3.0f, 3.0f, 3.0f));

				m_drawData.vertices = indexedList.vertices;
				m_drawData.indices = indexedList.indices;
				m_drawData.vertexSizeInBytes = m_drawData.vertices.size() * sizeof(m_drawData.vertices[0]);
				m_drawData.indexSizeInBytes = m_drawData.indices.size() * sizeof(m_drawData.indices[0]);
				m_drawData.vertexStrideInBytes = sizeof(VertexStruct);

				m_drawData.vertexBuffer = std::move(std::make_shared<D3D12Buffer>(m_gfx, m_drawData.vertices.data(), m_drawData.vertexSizeInBytes));
				m_drawData.indexBuffer = std::move(std::make_shared<D3D12Buffer>(m_gfx, m_drawData.indices.data(), m_drawData.indexSizeInBytes));

				D3D12_DRAW_INDEXED_ARGUMENTS drawArgs = {};
				drawArgs.IndexCountPerInstance = m_drawData.indices.size();
				drawArgs.InstanceCount = 1;
				drawArgs.StartIndexLocation = 0;
				drawArgs.BaseVertexLocation = 0;
				drawArgs.StartInstanceLocation = 0;

				m_drawData.drawIndirectBuffer = std::move(std::make_shared<D3D12Buffer>(m_gfx, &drawArgs, sizeof(drawArgs)));
			}

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

			UINT num32BitConstants[2] = { 11, 2 * sizeof(XMMATRIX) / 4 };  

			PipelineDescription pipelineDesc{};  
			pipelineDesc.numRenderTargets = m_renderTargets.size();  
			pipelineDesc.renderTargetFormats = renderTargetFormats;  
			pipelineDesc.numConstants = 2;  
			pipelineDesc.num32BitConstants = num32BitConstants;  
			pipelineDesc.numStaticSamplers = 1;  
			pipelineDesc.depthStencilMode = Mode::DepthFirst;  
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader,  L"Skybox_VS.hlsl" };  
			pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Skybox_PS.hlsl" };  

			m_rootSignature = std::move(std::make_unique<RootSignature>(m_gfx, pipelineDesc));  
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(m_gfx, pipelineDesc));  
		}  
		void Execute() noexcept(!IS_DEBUG) override  
		{
			m_renderTargets[0] = m_gfx.GetResourcePtr(m_gfx.GetCurrentBackBufferIndex());  

			m_rootSignature->Bind();  
			m_pipelineStateObject->Bind();  

			m_gfx.SetRenderTargets(m_renderTargets, m_depthStencil);  
			m_gfx.Set32BitRootConstants(0, 11, &RenderGraph::m_frameData);  
			m_gfx.SetPrimitiveTopology(m_pipelineStateObject->GetTopologyType());  

			Draw(m_drawData, true);
		}  

	private:  
		ILMesh::DrawData m_drawData{};
	};  
}