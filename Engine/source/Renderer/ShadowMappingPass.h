#pragma once
#include "RenderPass.h"
#include "RenderMath.h"
#include "CameraContainer.h"

namespace Renderer
{
	class ShadowMappingPass : public RenderPass
	{
	public:
		ShadowMappingPass(D3D12RHI& gfx, std::string name)
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

				UINT num32BitConstants[2] = { (sizeof(XMMATRIX) / 4) * 3 , 2};

				PipelineDescription shadowMapPipelineDesc{};
				shadowMapPipelineDesc.numConstants = 2;
				shadowMapPipelineDesc.num32BitConstants = num32BitConstants;
				shadowMapPipelineDesc.shadowMapping = true;
				shadowMapPipelineDesc.numElements = vec.size();
				shadowMapPipelineDesc.inputElementDescs = inputElementDescs;
				shadowMapPipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, L"Shadow_VS.hlsl" };
				shadowMapPipelineDesc.depthUsage = DepthUsage::ShadowDepth;

				m_rootSignature = std::move(std::make_unique<RootSignature>(gfx, shadowMapPipelineDesc));
				m_pipelineStateObject = std::move(std::make_unique<PipelineState>(gfx, shadowMapPipelineDesc));
			}

			m_pDepthCube = std::make_shared<DepthCubeMapTextureBuffer>(gfx, m_size);
			RenderGraph::m_shadowDepth360Handle = gfx.LoadResource(m_pDepthCube, ResourceType::CubeMapTexture);
			RegisterSource(DirectBindableSource<DepthCubeMapTextureBuffer>::Make("map", m_pDepthCube));

			m_depthStencil = std::move(m_pDepthCube->GetDepthBuffer(0));

			m_depthOnlyPass = true;
		}
		XMMATRIX Get360CameraMatrix(UINT directionIndex, XMFLOAT3& position) const noexcept(!IS_DEBUG)
		{
			const auto cameraPosition = XMLoadFloat3(&position);

			switch (directionIndex)
			{
			case 0: // +x			
				return XMMatrixLookAtLH(cameraPosition, cameraPosition + XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
				break;
			case 1: // -x			
				return XMMatrixLookAtLH(cameraPosition, cameraPosition + XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
				break;
			case 2: // +y			
				return XMMatrixLookAtLH(cameraPosition, cameraPosition + XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
				break;
			case 3: // -y			
				return XMMatrixLookAtLH(cameraPosition, cameraPosition + XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
				break;
			case 4: // +z			
				return XMMatrixLookAtLH(cameraPosition, cameraPosition + XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
				break;
			case 5: // -z			
				return XMMatrixLookAtLH(cameraPosition, cameraPosition + XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
				break;
			default:
				return XMMATRIX();
				break;
			}
		}
		XMMATRIX Get360ProjectionMatrix() const noexcept(!IS_DEBUG)
		{
			return XMMatrixPerspectiveFovLH(PI / 2.0f, 1.0f, 0.5f, 100.0f);
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

				Drawable::m_cameraMatrix = Get360CameraMatrix(i, ILMaterial::m_lightPosition);
				Drawable::m_projectionMatrix = Get360ProjectionMatrix();

				RenderPass::Execute(gfx);
			}
			gfx.ResizeScreenSpace(initWidth, initHeight);
		}

	private:
		ResourceHandle m_depthCubeHandle;
		VertexLayout m_vtxLayout;
		static constexpr UINT m_size = 1000;
		std::shared_ptr<DepthCubeMapTextureBuffer> m_pDepthCube;
	};
}