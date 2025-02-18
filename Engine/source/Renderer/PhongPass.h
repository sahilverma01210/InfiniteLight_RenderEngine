#pragma once
#include "RenderPass.h"
#include "CameraContainer.h"

namespace Renderer
{
	class PhongPass : public RenderPass
	{
	public:
		PhongPass(D3D12RHI& gfx, std::string name, CameraContainer& cameraContainer)
			:
			RenderPass(std::move(name)),
			m_cameraContainer(cameraContainer)
		{
			{
				m_vtxLayout.Append(VertexLayout::Position3D);
				m_vtxLayout.Append(VertexLayout::Normal);
				m_vtxLayout.Append(VertexLayout::Texture2D);
				m_vtxLayout.Append(VertexLayout::Tangent);
				m_vtxLayout.Append(VertexLayout::Bitangent);

				// Define the vertex input layout.
				std::vector<D3D12_INPUT_ELEMENT_DESC> vec = m_vtxLayout.GetD3DLayout();
				D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

				for (size_t i = 0; i < vec.size(); ++i) {
					inputElementDescs[i] = vec[i];
				}

				PipelineDescription phongPipelineDesc{};
				phongPipelineDesc.numConstants = 1;
				phongPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
				phongPipelineDesc.numConstantBufferViews = 3;
				phongPipelineDesc.numShaderResourceViews = 4;
				phongPipelineDesc.numSamplers = 2;
				phongPipelineDesc.backFaceCulling = true;
				phongPipelineDesc.numElements = vec.size();
				phongPipelineDesc.inputElementDescs = inputElementDescs;
				phongPipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Phong_VS.hlsl" };
				phongPipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Phong_PS.hlsl" };

				m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, phongPipelineDesc));
				m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, phongPipelineDesc));
			}

			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.m_textureManager.GetTexturePtr(3));
			m_pShadowMap = std::dynamic_pointer_cast<DepthCubeMapTextureBuffer>(gfx.m_textureManager.GetTexturePtr(4));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			m_cameraContainer.GetActiveCamera().Update();

			gfx.TransitionResource(m_pShadowMap->GetBuffer(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			m_renderTarget = std::dynamic_pointer_cast<RenderTarget>(gfx.m_textureManager.GetTexturePtr(gfx.GetCurrentBackBufferIndex() + 1));
			RenderPass::Execute(gfx);
			gfx.TransitionResource(m_pShadowMap->GetBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}

	private:
		VertexLayout m_vtxLayout;
		CameraContainer& m_cameraContainer;
		std::shared_ptr<DepthCubeMapTextureBuffer> m_pShadowMap;
	};
}