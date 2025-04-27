#pragma once
#include "ILMaterial.h"

struct aiMaterial;

namespace Renderer
{
	class ImportMaterial : public ILMaterial
	{
		friend class PointLight;

	public:
		__declspec(align(256u)) struct ImportMatHandles
		{
			ResourceHandle texConstIdx;
			ResourceHandle diffTexIdx;
			ResourceHandle normTexIdx;
			ResourceHandle specTexIdx;
			ResourceHandle solidConstIdx;
		};

	private:
		__declspec(align(256u)) struct SurfaceProps
		{
			alignas(16) XMFLOAT3 materialColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
			alignas(16) XMFLOAT3 specularColor = XMFLOAT3();
			float specularGloss = 0.0f;
			float normalMapWeight = 0.0f;
			bool useDiffuseAlpha = false;
			bool useGlossAlpha = false;
			bool useDiffuseMap = false;
			bool useNormalMap = false;
			bool useSpecularMap = false;
		};

		__declspec(align(256u)) struct SolidCB
		{
			XMFLOAT3 materialColor;
		};

	public:
		ImportMaterial() = default;
		ImportMaterial(D3D12RHI& gfx, const aiMaterial& material, const std::filesystem::path& path, bool enablePostProcessing, std::mutex &mutex) noexcept(!IS_DEBUG)
			: m_gfx(gfx)
		{
			// shadow map technique
			Technique map{ "ShadowMap", true };
			map.passNames.push_back("shadowMap");
			m_techniques.push_back(std::move(map));

			// gbuffer creation technique
			Technique gbuffer{ "GBuffer", true };
			gbuffer.passNames.push_back("gbuffer");
			m_techniques.push_back(std::move(gbuffer));

			if (enablePostProcessing)
			{
				// outline technique
				Technique outline{ "Outline", false };
				outline.passNames.push_back("outlineDraw");
				m_techniques.push_back(std::move(outline));
			}

			// Load Texture Images
			{
				const auto rootPath = path.parent_path().string() + "\\";

				// diffuse
				{
					if (aiString diffFileName; material.GetTexture(aiTextureType_DIFFUSE, 0, &diffFileName) == aiReturn_SUCCESS)
					{
						m_surfaceProps.useDiffuseMap = true;

						std::string diffPath = rootPath + diffFileName.C_Str();
						m_importMatHandles.diffTexIdx = gfx.LoadResource(MeshTextureBuffer::Resolve(gfx, diffPath), ResourceType::Texture);
					}
					else
					{
						aiColor3D color = { 0.45f,0.45f,0.85f };
						material.Get(AI_MATKEY_COLOR_DIFFUSE, color);
						m_surfaceProps.materialColor = reinterpret_cast<XMFLOAT3&>(color);
					}
				}
				// normal
				{
					if (aiString normFileName; material.GetTexture(aiTextureType_NORMALS, 0, &normFileName) == aiReturn_SUCCESS)
					{
						m_surfaceProps.useNormalMap = true;

						std::string normPath = rootPath + normFileName.C_Str();
						m_importMatHandles.normTexIdx = gfx.LoadResource(MeshTextureBuffer::Resolve(gfx, normPath), ResourceType::Texture);
					}
				}
				// specular
				{
					aiColor3D color = { 0.18f,0.18f,0.18f };
					material.Get(AI_MATKEY_COLOR_SPECULAR, color);

					float gloss = 8.0f;
					material.Get(AI_MATKEY_SHININESS, gloss);

					m_surfaceProps.specularColor = reinterpret_cast<XMFLOAT3&>(color);
					m_surfaceProps.specularGloss = gloss;

					if (aiString specFileName; material.GetTexture(aiTextureType_SPECULAR, 0, &specFileName) == aiReturn_SUCCESS)
					{
						m_surfaceProps.useSpecularMap = true;

						std::string specPath = rootPath + specFileName.C_Str();
						m_importMatHandles.specTexIdx = gfx.LoadResource(MeshTextureBuffer::Resolve(gfx, specPath), ResourceType::Texture);
					}
				}
			}

			// Load Constant Buffers
			{
				m_data.materialColor = XMFLOAT3{ 1.0f,0.4f,0.4f };
				if (m_importMatHandles.diffTexIdx) m_surfaceProps.useDiffuseAlpha = dynamic_cast<MeshTextureBuffer&>(gfx.GetResource(m_importMatHandles.diffTexIdx)).HasAlpha();
				if (m_importMatHandles.normTexIdx) m_surfaceProps.normalMapWeight = 1.0f;
				if (m_importMatHandles.specTexIdx) m_surfaceProps.useGlossAlpha = dynamic_cast<MeshTextureBuffer&>(gfx.GetResource(m_importMatHandles.specTexIdx)).HasAlpha();

				m_surfacePropsCB = std::make_shared<ConstantBuffer>(gfx, sizeof(m_surfaceProps), &m_surfaceProps);
				m_pSolidCBuf = std::make_shared<ConstantBuffer>(gfx, sizeof(m_data), &m_data);

				m_importMatHandles.texConstIdx = gfx.LoadResource(m_surfacePropsCB, ResourceType::Constant);
				m_importMatHandles.solidConstIdx = gfx.LoadResource(m_pSolidCBuf, ResourceType::Constant);
			}

			m_materialHandle = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_importMatHandles), &m_importMatHandles), ResourceType::Constant);
		}
		UINT getID() const override {
			return getTypeID<ImportMaterial>();
		}
		void ToggleEffect(std::string name, bool enabled)
		{
			m_surfaceProps.materialColor = XMFLOAT3{ 1.0f,0.4f,0.4f };
			m_surfacePropsCB->Update(m_gfx, &m_surfaceProps);
		}

	private:
		D3D12RHI& m_gfx;
		SurfaceProps m_surfaceProps;
		SolidCB m_data;
		ImportMatHandles m_importMatHandles{};
		std::shared_ptr<ConstantBuffer> m_surfacePropsCB;
		std::shared_ptr<ConstantBuffer> m_pSolidCBuf;
	};
}