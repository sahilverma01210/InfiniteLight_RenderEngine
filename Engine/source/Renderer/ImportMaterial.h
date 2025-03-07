#pragma once
#include "ILMaterial.h"

struct aiMaterial;

namespace Renderer
{
	class ImportMaterial : public ILMaterial
	{
		friend class PointLight;

		__declspec(align(256u)) struct ImportMatHandles
		{
			ResourceHandle phongLightShadowIdx;
			ResourceHandle phongLightConstIdx;
			ResourceHandle phongTexConstIdx;
			ResourceHandle phongShadowTexIdx;
			ResourceHandle phongDiffTexIdx;
			ResourceHandle phongNormTexIdx;
			ResourceHandle phongSpecTexIdx;
			ResourceHandle solidConstIdx;
		};

		__declspec(align(256u)) struct PhongCB
		{
			alignas(16) XMFLOAT3 materialColor = XMFLOAT3();
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
		ImportMaterial(D3D12RHI& gfx, const aiMaterial& material, const std::filesystem::path& path, std::mutex &mutex) noexcept(!IS_DEBUG)
		{
			// shadow map technique
			Technique map{ "ShadowMap",Channel::shadow,true };
			{
				// 1. Shadow Map Step
				Step draw("shadowMap");
				{
				}
				map.AddStep(std::move(draw));
			}
			m_techniques.push_back(std::move(map));

			// phong technique
			Technique phong{ "Phong",Channel::main };
			{
				// 1. Phong Shading Step
				Step step("phong_shading");
				{
					UINT numSRVDescriptors = 0;

					const auto rootPath = path.parent_path().string() + "\\";

					// common (pre)
					std::vector<SHADER_MACRO> macros;

					// Load Texture Images
					{
						//std::lock_guard<std::mutex> guard(mutex);

						// shadow
						{
							m_importMatHandles.phongShadowTexIdx = 3;
						}
						// diffuse
						{
							if (aiString diffFileName; material.GetTexture(aiTextureType_DIFFUSE, 0, &diffFileName) == aiReturn_SUCCESS)
							{
								m_phongData.useDiffuseMap = true;

								std::string diffPath = rootPath + diffFileName.C_Str();
								m_importMatHandles.phongDiffTexIdx = gfx.LoadResource(MeshTextureBuffer::Resolve(gfx, diffPath), ResourceType::Texture);

								macros.push_back({ L"USE_DIFFUSE_MAP", L"1" });
								numSRVDescriptors++;
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

								macros.push_back({ L"USE_NORMAL_MAP", L"1" });
								numSRVDescriptors++;
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

								macros.push_back({ L"USE_SPECULAR_MAP", L"1" });
								numSRVDescriptors++;
							}
						}
					}

					// Add Resources
					{
						// Add Constants
						{
							if (m_importMatHandles.phongDiffTexIdx) m_phongData.useDiffuseAlpha = dynamic_cast<MeshTextureBuffer&>(gfx.GetResource(m_importMatHandles.phongDiffTexIdx)).HasAlpha();
							if (m_importMatHandles.phongNormTexIdx) m_phongData.normalMapWeight = 1.0f;
							if (m_importMatHandles.phongSpecTexIdx) m_phongData.useGlossAlpha = dynamic_cast<MeshTextureBuffer&>(gfx.GetResource(m_importMatHandles.phongSpecTexIdx)).HasAlpha();

							m_importMatHandles.phongLightShadowIdx = gfx.LoadResource(m_lightShadowBindable, ResourceType::Constant);
							m_importMatHandles.phongLightConstIdx = gfx.LoadResource(m_lightBindable, ResourceType::Constant);
							m_importMatHandles.phongTexConstIdx = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_phongData), &m_phongData), ResourceType::Constant);
						}
					}
				}
				phong.AddStep(std::move(step));
			}
			m_techniques.push_back(std::move(phong));

			if (m_postProcessEnabled)
			{
				// outline technique
				Technique outline{ "Outline",Channel::main,false };
				{
					// 1. Outline Mask Step
					Step mask("outlineMask");
					{
					}
					outline.AddStep(std::move(mask));

					// 2. Outline Draw Step
					Step draw("outlineDraw");
					{
						// Add Resources
						{
							// Add Constants
							{
								SolidCB data = { XMFLOAT3{ 1.0f,0.4f,0.4f } };							
								m_importMatHandles.solidConstIdx = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(data), static_cast<const void*>(&data)), ResourceType::Constant);
							}
						}
					}
					outline.AddStep(std::move(draw));
				}
				m_techniques.push_back(std::move(outline));

				m_materialHandle = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_importMatHandles), static_cast<const void*>(&m_importMatHandles)), ResourceType::Constant);
			}
		}
		UINT getID() const override {
			return getTypeID<ImportMaterial>();
		}

	private:
		PhongCB m_phongData;
		static std::shared_ptr<ConstantBuffer> m_lightBindable;
		static std::shared_ptr<ConstantBuffer> m_lightShadowBindable;
		ImportMatHandles m_importMatHandles{};
	};
}