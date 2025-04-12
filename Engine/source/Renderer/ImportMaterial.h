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
			ResourceHandle phongLightConstIdx;
			ResourceHandle phongTexConstIdx;
			ResourceHandle phongShadowTexIdx;
			ResourceHandle phongDiffTexIdx;
			ResourceHandle phongNormTexIdx;
			ResourceHandle phongSpecTexIdx;
			ResourceHandle solidConstIdx;
		};

	private:
		__declspec(align(256u)) struct PhongCB
		{
			alignas(16) XMFLOAT3 materialColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
			alignas(16) XMFLOAT3 specularColor = XMFLOAT3();
			float specularGloss = 0.0f;
			float specularWeight = 0.0f;
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

			// phong technique
			Technique phong{ "Phong", true };
			phong.passNames.push_back("phongShading");
			m_techniques.push_back(std::move(phong));

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

				// shadow
				{
					m_importMatHandles.phongShadowTexIdx = RenderGraph::m_shadowDepth360Handle;
				}
				// diffuse
				{
					if (aiString diffFileName; material.GetTexture(aiTextureType_DIFFUSE, 0, &diffFileName) == aiReturn_SUCCESS)
					{
						m_phongData.useDiffuseMap = true;

						std::string diffPath = rootPath + diffFileName.C_Str();
						m_importMatHandles.phongDiffTexIdx = gfx.LoadResource(MeshTextureBuffer::Resolve(gfx, diffPath), ResourceType::Texture);
					}
					else
					{
						aiColor3D color = { 0.45f,0.45f,0.85f };
						material.Get(AI_MATKEY_COLOR_DIFFUSE, color);
						m_phongData.materialColor = reinterpret_cast<XMFLOAT3&>(color);
					}
				}
				// normal
				{
					if (aiString normFileName; material.GetTexture(aiTextureType_NORMALS, 0, &normFileName) == aiReturn_SUCCESS)
					{
						m_phongData.useNormalMap = true;

						std::string normPath = rootPath + normFileName.C_Str();
						m_importMatHandles.phongNormTexIdx = gfx.LoadResource(MeshTextureBuffer::Resolve(gfx, normPath), ResourceType::Texture);
					}
				}
				// specular
				{
					aiColor3D color = { 0.18f,0.18f,0.18f };
					material.Get(AI_MATKEY_COLOR_SPECULAR, color);

					float gloss = 8.0f;
					material.Get(AI_MATKEY_SHININESS, gloss);

					m_phongData.specularColor = reinterpret_cast<XMFLOAT3&>(color);
					m_phongData.specularGloss = gloss;
					m_phongData.specularWeight = 1.0f;

					if (aiString specFileName; material.GetTexture(aiTextureType_SPECULAR, 0, &specFileName) == aiReturn_SUCCESS)
					{
						m_phongData.useSpecularMap = true;

						std::string specPath = rootPath + specFileName.C_Str();
						m_importMatHandles.phongSpecTexIdx = gfx.LoadResource(MeshTextureBuffer::Resolve(gfx, specPath), ResourceType::Texture);
					}
				}
			}

			// Load Constant Buffers
			{
				m_data.materialColor = XMFLOAT3{ 1.0f,0.4f,0.4f };
				if (m_importMatHandles.phongDiffTexIdx) m_phongData.useDiffuseAlpha = dynamic_cast<MeshTextureBuffer&>(gfx.GetResource(m_importMatHandles.phongDiffTexIdx)).HasAlpha();
				if (m_importMatHandles.phongNormTexIdx) m_phongData.normalMapWeight = 1.0f;
				if (m_importMatHandles.phongSpecTexIdx) m_phongData.useGlossAlpha = dynamic_cast<MeshTextureBuffer&>(gfx.GetResource(m_importMatHandles.phongSpecTexIdx)).HasAlpha();

				m_pPhongCBuf = std::make_shared<ConstantBuffer>(gfx, sizeof(m_phongData), &m_phongData);
				m_pSolidCBuf = std::make_shared<ConstantBuffer>(gfx, sizeof(m_data), &m_data);

				m_importMatHandles.phongLightConstIdx = ILMaterial::m_lightHandle;
				m_importMatHandles.phongTexConstIdx = gfx.LoadResource(m_pPhongCBuf, ResourceType::Constant);
				m_importMatHandles.solidConstIdx = gfx.LoadResource(m_pSolidCBuf, ResourceType::Constant);
			}

			m_materialHandle = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_importMatHandles), &m_importMatHandles), ResourceType::Constant);
		}
		UINT getID() const override {
			return getTypeID<ImportMaterial>();
		}
		void ToggleEffect(std::string name, bool enabled)
		{
			m_phongData.materialColor = XMFLOAT3{ 1.0f,0.4f,0.4f };
			m_pPhongCBuf->Update(m_gfx, &m_phongData);
		}

	private:
		D3D12RHI& m_gfx;
		PhongCB m_phongData;
		SolidCB m_data;
		ImportMatHandles m_importMatHandles{};
		std::shared_ptr<ConstantBuffer> m_pPhongCBuf;
		std::shared_ptr<ConstantBuffer> m_pSolidCBuf;
	};
}