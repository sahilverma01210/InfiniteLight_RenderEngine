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
					// Add Resources & Samplers
					{
						DescriptorTable::TableParams params;
						params.resourceParameterIndex = 1;
						params.samplerParameterIndex = 2;
						params.numCbvSrvUavDescriptors = 1;
						params.numSamplerDescriptors = 1;

						std::shared_ptr<DescriptorTable> descriptorTable = std::make_shared<DescriptorTable>(gfx, params);

						// Add Textures
						{
							TextureHandle textureHandle = gfx.m_textureManager.LoadTexture(std::make_shared<CubeMapTextureBuffer>(gfx, L"data\\textures\\SpaceBox"));
							descriptorTable->AddShaderResourceView(gfx, textureHandle, false, true);
						}

						// Add Samplers
						{
							D3D12_SAMPLER_DESC sampler{};
							sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
							sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							descriptorTable->AddSampler(gfx, &sampler);
						}

						only.AddBindable(std::move(descriptorTable));
					}
				}
				skybox.AddStep(std::move(only));
			}
			m_techniques.push_back(std::move(skybox));
		}
	};
}