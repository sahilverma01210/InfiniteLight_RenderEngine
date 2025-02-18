#pragma once
#include "RenderPass.h"
#include "RenderMath.h"
#include "CameraContainer.h"

namespace Renderer
{
	class ShadowMappingPass : public RenderPass
	{
	public:
		ShadowMappingPass(D3D12RHI& gfx, std::string name, CameraContainer& cameraContainer)
			:
			RenderPass(std::move(name)),
			m_cameraContainer(cameraContainer)
		{
			{
				m_vtxLayout.Append(VertexLayout::Position3D);

				// Define the vertex input layout.
				std::vector<D3D12_INPUT_ELEMENT_DESC> vec = m_vtxLayout.GetD3DLayout();
				D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

				for (size_t i = 0; i < vec.size(); ++i) {
					inputElementDescs[i] = vec[i];
				}

				PipelineDescription shadowMapPipelineDesc{};
				shadowMapPipelineDesc.numConstants = 1;
				shadowMapPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
				shadowMapPipelineDesc.shadowMapping = true;
				shadowMapPipelineDesc.numElements = vec.size();
				shadowMapPipelineDesc.inputElementDescs = inputElementDescs;
				shadowMapPipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Shadow_VS.hlsl" };
				shadowMapPipelineDesc.depthUsage = DepthUsage::ShadowDepth;

				m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, shadowMapPipelineDesc));
				m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, shadowMapPipelineDesc));
			}

			m_pDepthCube = std::make_shared<DepthCubeMapTextureBuffer>(gfx, m_size);
			gfx.m_textureManager.LoadTexture(m_pDepthCube);
			RegisterSource(DirectBindableSource<DepthCubeMapTextureBuffer>::Make("map", m_pDepthCube));

			m_depthStencil = std::move(m_pDepthCube->GetDepthBuffer(0));

			m_depthOnlyPass = true;
		}
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override
		{
			UINT initWidth = gfx.GetWidth();
			UINT initHeight = gfx.GetHeight();

			gfx.ResizeScreenSpace(m_size, m_size);

			for (size_t i = 0; i < 6; i++)
			{
				m_depthStencil = std::move(m_pDepthCube->GetDepthBuffer(i));
				m_depthStencil->Clear(gfx);

				m_cameraContainer.GetLightingCamera().Update(true, i);

				RenderPass::Execute(gfx);
			}
			gfx.ResizeScreenSpace(initWidth, initHeight);
		}

	private:
		TextureHandle m_depthCubeHandle;
		VertexLayout m_vtxLayout;
		static constexpr UINT m_size = 1000;
		CameraContainer& m_cameraContainer;
		std::shared_ptr<DepthCubeMapTextureBuffer> m_pDepthCube;
	};
}