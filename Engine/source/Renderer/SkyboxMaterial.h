#pragma once
#include "ILMaterial.h"

namespace Renderer
{
	class SkyboxMaterial : public ILMaterial
	{
		__declspec(align(256u)) struct SkyboxMatHandles
		{
			ResourceHandle cubeMapTexIdx;
		};

	public:
		SkyboxMaterial(D3D12RHI& gfx, VertexLayout layout) noexcept(!IS_DEBUG)
		{
			Technique skybox{ "skybox", true };
			skybox.passNames.push_back("skybox");
			m_techniques.push_back(std::move(skybox));

			m_skyboxMatHandles.cubeMapTexIdx = gfx.LoadResource(std::make_shared<CubeMapTexture>(gfx, L"data\\textures\\SpaceBox"));
			m_materialHandle = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &m_skyboxMatHandles, sizeof(m_skyboxMatHandles)));
		}
		UINT getID() const override {
			return getTypeID<SkyboxMaterial>();
		}

	private:
		SkyboxMatHandles m_skyboxMatHandles{};
	};
}