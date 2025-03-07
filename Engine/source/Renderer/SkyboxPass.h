#pragma once
#include "RenderPass.h"
#include "Camera.h"
#include "Shapes.h"

namespace Renderer
{
	class SkyboxPass : public RenderPass
	{
	public:
		SkyboxPass(D3D12RHI& gfx, std::string name)
			:
			RenderPass(std::move(name))
		{
			{
				m_vtxLayout.Append(VertexLayout::Position3D);

				// Define the vertex input layout.
				std::vector<D3D12_INPUT_ELEMENT_DESC> vec = m_vtxLayout.GetD3DLayout();
				D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

				for (size_t i = 0; i < vec.size(); ++i) {
					inputElementDescs[i] = vec[i];
				}

				UINT num32BitConstants[2] = { (sizeof(XMMATRIX) / 4) * 3 , 2 };

				CD3DX12_STATIC_SAMPLER_DESC* samplers = new CD3DX12_STATIC_SAMPLER_DESC[1];
				// define static sampler 
				CD3DX12_STATIC_SAMPLER_DESC staticSampler{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
				staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				staticSampler.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
				staticSampler.MipLODBias = 0.0f;
				staticSampler.MinLOD = 0.0f;
				staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
				samplers[0] = staticSampler;

				PipelineDescription skyboxPipelineDesc{};
				skyboxPipelineDesc.numConstants = 2;
				skyboxPipelineDesc.num32BitConstants = num32BitConstants;
				skyboxPipelineDesc.numStaticSamplers = 1;
				skyboxPipelineDesc.staticSamplers = samplers;
				skyboxPipelineDesc.depthStencilMode = Mode::DepthFirst;
				skyboxPipelineDesc.numElements = vec.size();
				skyboxPipelineDesc.inputElementDescs = inputElementDescs;
				skyboxPipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader,  L"Skybox_VS.hlsl" };
				skyboxPipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, L"Skybox_PS.hlsl" };

				m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, skyboxPipelineDesc));
				m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, skyboxPipelineDesc));
			}

			m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(gfx.GetResourcePtr(2));
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			m_renderTarget = std::dynamic_pointer_cast<RenderTarget>(gfx.GetResourcePtr(gfx.GetCurrentBackBufferIndex()));
			RenderPass::Execute(gfx);
		}

	private:
		VertexLayout m_vtxLayout;
	};
}