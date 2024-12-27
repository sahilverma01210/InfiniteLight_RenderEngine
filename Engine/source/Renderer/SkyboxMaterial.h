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

						PipelineDescription skyboxPipelineDesc{};

						skyboxPipelineDesc.numConstants = 1;
						skyboxPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4);
						skyboxPipelineDesc.numShaderResourceViews = 1;
						skyboxPipelineDesc.numSamplers = 1;
						skyboxPipelineDesc.depthStencilMode = Mode::DepthFirst;
						skyboxPipelineDesc.numElements = vec.size();
						skyboxPipelineDesc.inputElementDescs = inputElementDescs;
						skyboxPipelineDesc.pixelShader = pixelShader;
						skyboxPipelineDesc.vertexShader = vertexShader;

						m_pipelineDesc["skybox"] = skyboxPipelineDesc;
					}

					DescriptorTable::TableParams params;
					params.resourceParameterIndex = 1;
					params.samplerParameterIndex = 2;
					params.numCbvSrvUavDescriptors = 1;
					params.numSamplerDescriptors = 1;

					std::shared_ptr<DescriptorTable> descriptorTable = std::make_shared<DescriptorTable>(gfx, params);

					std::shared_ptr<CubeMapTextureBuffer> texture = std::make_shared<CubeMapTextureBuffer>(gfx, L"data\\textures\\SpaceBox");
					descriptorTable->AddShaderResourceView(gfx, texture->GetBuffer(), false, true);

					D3D12_SAMPLER_DESC sampler{};
					sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
					sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
					sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
					sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
					descriptorTable->AddSampler(gfx, &sampler);

					only.AddBindable(std::move(texture));
					only.AddBindable(std::move(descriptorTable));
				}
				skybox.AddStep(std::move(only));
			}
			m_techniques.push_back(std::move(skybox));
		}
	};
}