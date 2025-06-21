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
			ResourceHandle diffuseIdx;
			ResourceHandle normalIdx;
			ResourceHandle roughnessMetallicIdx;
			ResourceHandle materialConstIdx;
			ResourceHandle solidConstIdx;
		};

	private:
		__declspec(align(256u)) struct MaterialConstants
		{
			alignas(16) XMFLOAT3 pbrBaseColorFactor = XMFLOAT3(1.0f, 1.0f, 1.0f);
			float pbrMetallicFactor = 1.0f;
			float pbrRoughnessFactor = 1.0f;
			float gltfAlphaCutoff = 0.5f;
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

			aiReturn aiDiffuse, aiNormal, aiMetallicRoughness;
			aiString diffFileName, normFileName, metalRoughFileName;

			// Load Texture Images
			{
				const auto rootPath = path.parent_path().string() + "\\";

				// diffuse
				{
					aiDiffuse = material.GetTexture(aiTextureType_DIFFUSE, 0, &diffFileName);
					m_importMatHandles.diffuseIdx = gfx.LoadResource(std::make_shared<MeshTexture>(gfx, aiDiffuse == aiReturn_SUCCESS ? rootPath + diffFileName.C_Str() : "NULL_TEX", true));
				}
				// normal
				{
					aiNormal = material.GetTexture(aiTextureType_NORMALS, 0, &normFileName);
					m_importMatHandles.normalIdx = gfx.LoadResource(std::make_shared<MeshTexture>(gfx, aiNormal == aiReturn_SUCCESS ? rootPath + normFileName.C_Str() : "NULL_TEX"));
				}
				// metallic roughness
				{
					aiMetallicRoughness = material.GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &metalRoughFileName);
					m_importMatHandles.roughnessMetallicIdx = gfx.LoadResource(std::make_shared<MeshTexture>(gfx, aiMetallicRoughness == aiReturn_SUCCESS ? rootPath + metalRoughFileName.C_Str() : "NULL_TEX"));
				}
			}

			// Load Constant Buffers
			{
				m_data.materialColor = XMFLOAT3{ 1.0f,0.4f,0.4f };
				m_pSolidCBuf = std::make_shared<D3D12Buffer>(gfx, &m_data, sizeof(m_data));

				m_importMatHandles.solidConstIdx = gfx.LoadResource(m_pSolidCBuf);

				aiColor4D baseColor(1, 1, 1, 1);

				material.Get(AI_MATKEY_GLTF_ALPHACUTOFF, m_materialConstants.gltfAlphaCutoff);
				material.Get(AI_MATKEY_BASE_COLOR, baseColor);
				m_materialConstants.pbrBaseColorFactor.x = baseColor.r;
				m_materialConstants.pbrBaseColorFactor.y = baseColor.g;
				m_materialConstants.pbrBaseColorFactor.z = baseColor.b;
				material.Get(AI_MATKEY_METALLIC_FACTOR, m_materialConstants.pbrMetallicFactor);
				material.Get(AI_MATKEY_ROUGHNESS_FACTOR, m_materialConstants.pbrRoughnessFactor);

				m_materialConstCB = std::make_shared<D3D12Buffer>(gfx, &m_materialConstants, sizeof(m_materialConstants));
				m_importMatHandles.materialConstIdx = gfx.LoadResource(m_materialConstCB);
			}

			m_materialHandle = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &m_importMatHandles, sizeof(m_importMatHandles)));
		}
		UINT getID() const override {
			return getTypeID<ImportMaterial>();
		}
		void ToggleEffect(std::string name, bool enabled)
		{

		}

	private:
		D3D12RHI& m_gfx;
		MaterialConstants m_materialConstants;
		SolidCB m_data;
		ImportMatHandles m_importMatHandles{};
		std::shared_ptr<D3D12Buffer> m_materialConstCB;
		std::shared_ptr<D3D12Buffer> m_pSolidCBuf;
	};
}