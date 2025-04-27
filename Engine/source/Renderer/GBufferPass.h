#pragma once
#include "RenderPass.h"
#include "CameraContainer.h"

namespace Renderer
{
	class GBufferPass : public RenderPass
	{
	public:
		GBufferPass(D3D12RHI& gfx, std::string name, CameraContainer& cameraContainer)
			:
			RenderPass(std::move(name)),
			m_cameraContainer(cameraContainer)
		{
			m_renderTargets.resize(4);
			m_renderTargets[0] = std::make_shared<RenderTarget>(gfx, gfx.GetWidth(), gfx.GetHeight(), DXGI_FORMAT_R32G32B32A32_FLOAT);
			m_renderTargets[1] = std::make_shared<RenderTarget>(gfx, gfx.GetWidth(), gfx.GetHeight(), DXGI_FORMAT_R32G32B32A32_FLOAT);
			m_renderTargets[2] = std::make_shared<RenderTarget>(gfx, gfx.GetWidth(), gfx.GetHeight(), DXGI_FORMAT_R32G32B32A32_FLOAT);
			m_renderTargets[3] = std::make_shared<RenderTarget>(gfx, gfx.GetWidth(), gfx.GetHeight(), DXGI_FORMAT_R32G32B32A32_FLOAT);
			RenderGraph::m_renderTargetHandles["Position"] = gfx.LoadResource(m_renderTargets[0], ResourceType::Texture);
			RenderGraph::m_renderTargetHandles["Diffuse"] = gfx.LoadResource(m_renderTargets[1], ResourceType::Texture);
			RenderGraph::m_renderTargetHandles["Normal"] = gfx.LoadResource(m_renderTargets[2], ResourceType::Texture);
			RenderGraph::m_renderTargetHandles["Specular"] = gfx.LoadResource(m_renderTargets[3], ResourceType::Texture);
			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.GetResourcePtr(RenderGraph::m_depthStencilHandle));
			m_pShadowMap = std::dynamic_pointer_cast<DepthCubeMapTextureBuffer>(gfx.GetResourcePtr(RenderGraph::m_shadowDepth360Handle));

			CreatePSO(gfx);
		}
		void CreatePSO(D3D12RHI& gfx)
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

			DXGI_FORMAT* renderTargetFormats = new DXGI_FORMAT[m_renderTargets.size()];

			for (size_t i = 0; i < m_renderTargets.size(); i++) {
				renderTargetFormats[i] = m_renderTargets[i]->GetFormat();
			}

			UINT num32BitConstants[2] = { (sizeof(XMMATRIX) / 4) * 3 , 2 };

			CD3DX12_STATIC_SAMPLER_DESC* staticSamplers = new CD3DX12_STATIC_SAMPLER_DESC[2];

			CD3DX12_STATIC_SAMPLER_DESC sampler1{ 0 };
			sampler1.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			sampler1.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler1.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler1.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
			sampler1.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			staticSamplers[0] = sampler1;
			CD3DX12_STATIC_SAMPLER_DESC sampler2{ 1 };
			sampler2.Filter = D3D12_FILTER_ANISOTROPIC;
			sampler2.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			sampler2.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			sampler2.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
			sampler2.MipLODBias = 0.0f;
			sampler2.MinLOD = 0.0f;
			sampler2.MaxLOD = D3D12_FLOAT32_MAX;
			staticSamplers[1] = sampler2;

			PipelineDescription pipelineDesc{};
			pipelineDesc.numRenderTargets = m_renderTargets.size();
			pipelineDesc.renderTargetFormats = renderTargetFormats;
			pipelineDesc.numConstants = 2;
			pipelineDesc.num32BitConstants = num32BitConstants;
			pipelineDesc.numStaticSamplers = 2; // One extra Sampler for Shadow Texture.
			pipelineDesc.staticSamplers = staticSamplers;
			pipelineDesc.backFaceCulling = true;
			pipelineDesc.numElements = vec.size();
			pipelineDesc.inputElementDescs = inputElementDescs;
			pipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"GBuffer_VS.hlsl" };
			pipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"GBuffer_PS.hlsl" };

			m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
			m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			m_cameraContainer.GetActiveCamera().Update();

			gfx.TransitionResource(m_pShadowMap->GetBuffer(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			gfx.ClearResource(RenderGraph::m_renderTargetHandles["Position"], ResourceType::RenderTarget);
			gfx.ClearResource(RenderGraph::m_renderTargetHandles["Diffuse"], ResourceType::RenderTarget);
			gfx.ClearResource(RenderGraph::m_renderTargetHandles["Normal"], ResourceType::RenderTarget);
			gfx.ClearResource(RenderGraph::m_renderTargetHandles["Specular"], ResourceType::RenderTarget);
			RenderPass::Execute(gfx);
			gfx.TransitionResource(m_pShadowMap->GetBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}

	private:
		VertexLayout m_vtxLayout;
		CameraContainer& m_cameraContainer;
		std::shared_ptr<DepthCubeMapTextureBuffer> m_pShadowMap;
	};
}