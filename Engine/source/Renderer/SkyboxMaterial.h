#pragma once
#include "ILMaterial.h"

namespace Renderer
{
	class SkyboxMaterial : public ILMaterial
	{
	public:
		SkyboxMaterial(D3D12RHI& gfx, VertexLayout layout) noexcept(!IS_DEBUG)
		{
			Technique skybox{ "skybox", Channel::main};
			{
				Step only("skybox");
				{
					// Add Pipeline State Obejct
					{
						// Define the vertex input layout.
						std::vector<D3D12_INPUT_ELEMENT_DESC> vec = layout.GetD3DLayout();
						D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

						for (size_t i = 0; i < vec.size(); ++i) {
							inputElementDescs[i] = vec[i];
						}

						ID3DBlob* pixelShader;
						ID3DBlob* vertexShader;

						D3DCompileFromFile(GetAssetFullPath(L"Skybox_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", 0, 0, &pixelShader, nullptr);
						D3DCompileFromFile(GetAssetFullPath(L"Skybox_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", 0, 0, &vertexShader, nullptr);

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

						skyboxPipelineDesc.numConstants = 1;
						skyboxPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4);
						skyboxPipelineDesc.numShaderResourceViews = 1;
						skyboxPipelineDesc.numStaticSamplers = 1;
						skyboxPipelineDesc.staticSamplers = samplers;
						skyboxPipelineDesc.depthStencilMode = Mode::DepthFirst;
						skyboxPipelineDesc.numElements = vec.size();
						skyboxPipelineDesc.inputElementDescs = inputElementDescs;
						skyboxPipelineDesc.pixelShader = pixelShader;
						skyboxPipelineDesc.vertexShader = vertexShader;

						m_pipelineDesc["skybox"] = skyboxPipelineDesc;
					}

					only.AddBindable(std::make_shared<SkyboxTransformBuffer>(gfx, 0));

					std::shared_ptr<CubeMapTextureBuffer> texture = std::make_shared<CubeMapTextureBuffer>(gfx, L"data\\textures\\SpaceBox");

					std::shared_ptr<DescriptorTable> descriptorTable = std::make_shared<DescriptorTable>(gfx, 1, 1);
					descriptorTable->AddShaderResourceView(gfx, texture->GetBuffer(), false, true);

					only.AddBindable(std::move(texture));
					only.AddBindable(std::move(descriptorTable));
				}
				skybox.AddStep(std::move(only));
			}
			m_techniques.push_back(std::move(skybox));
		}
	};
}