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
						params.numCbvSrvUavDescriptors = 1;

						std::shared_ptr<DescriptorTable> descriptorTable = std::make_shared<DescriptorTable>(gfx, params);

						// Add Textures
						{
							TextureHandle textureHandle = gfx.m_textureManager.LoadTexture(std::make_shared<CubeMapTextureBuffer>(gfx, L"data\\textures\\SpaceBox"));
							descriptorTable->AddShaderResourceView(gfx, textureHandle, false, true);
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