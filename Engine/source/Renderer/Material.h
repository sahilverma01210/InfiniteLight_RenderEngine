#pragma once

struct aiMaterial;

namespace Renderer
{
	class Material
	{
	public:
		__declspec(align(256u)) struct MaterialDataHandles
		{
			ResourceHandle diffuseIdx;
			ResourceHandle normalIdx;
			ResourceHandle roughnessMetallicIdx;
			ResourceHandle materialConstIdx;
		};

	private:
		__declspec(align(256u)) struct MaterialConstants
		{
			alignas(16) XMFLOAT3 pbrBaseColorFactor = XMFLOAT3(1.0f, 1.0f, 1.0f);
			float pbrMetallicFactor = 1.0f;
			float pbrRoughnessFactor = 1.0f;
			float gltfAlphaCutoff = 0.5f;
		};

	public:
		Material() = default;
		Material(D3D12RHI& gfx, const aiMaterial& material, const std::filesystem::path& path, std::mutex &mutex) noexcept(!IS_DEBUG)
			: m_gfx(gfx)
		{
			aiReturn aiDiffuse, aiNormal, aiMetallicRoughness;
			aiString diffFileName, normFileName, metalRoughFileName;

			// Load Texture Images
			{
				const auto rootPath = path.parent_path().string() + "\\";

				// diffuse
				{
					aiDiffuse = material.GetTexture(aiTextureType_DIFFUSE, 0, &diffFileName);
					m_materialDataHandles.diffuseIdx = gfx.LoadResource(std::make_shared<MeshTexture>(gfx, aiDiffuse == aiReturn_SUCCESS ? rootPath + diffFileName.C_Str() : "NULL_TEX", true));
				}
				// normal
				{
					aiNormal = material.GetTexture(aiTextureType_NORMALS, 0, &normFileName);
					m_materialDataHandles.normalIdx = gfx.LoadResource(std::make_shared<MeshTexture>(gfx, aiNormal == aiReturn_SUCCESS ? rootPath + normFileName.C_Str() : "NULL_TEX"));
				}
				// metallic roughness
				{
					aiMetallicRoughness = material.GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &metalRoughFileName);
					m_materialDataHandles.roughnessMetallicIdx = gfx.LoadResource(std::make_shared<MeshTexture>(gfx, aiMetallicRoughness == aiReturn_SUCCESS ? rootPath + metalRoughFileName.C_Str() : "NULL_TEX"));
				}
			}

			// Load Constant Buffers
			{
				aiColor4D baseColor(1, 1, 1, 1);

				material.Get(AI_MATKEY_GLTF_ALPHACUTOFF, m_materialConstants.gltfAlphaCutoff);
				material.Get(AI_MATKEY_BASE_COLOR, baseColor);
				m_materialConstants.pbrBaseColorFactor.x = baseColor.r;
				m_materialConstants.pbrBaseColorFactor.y = baseColor.g;
				m_materialConstants.pbrBaseColorFactor.z = baseColor.b;
				material.Get(AI_MATKEY_METALLIC_FACTOR, m_materialConstants.pbrMetallicFactor);
				material.Get(AI_MATKEY_ROUGHNESS_FACTOR, m_materialConstants.pbrRoughnessFactor);

				m_materialConstCB = std::make_shared<D3D12Buffer>(gfx, &m_materialConstants, sizeof(m_materialConstants));
				m_materialDataHandles.materialConstIdx = gfx.LoadResource(m_materialConstCB);
			}

			m_materialHandle = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &m_materialDataHandles, sizeof(m_materialDataHandles)));
		}
		ResourceHandle GetMaterialHandle()
		{
			return m_materialHandle;
		}

	private:
		D3D12RHI& m_gfx;
		MaterialConstants m_materialConstants;
		MaterialDataHandles m_materialDataHandles{};
		std::shared_ptr<D3D12Buffer> m_materialConstCB;
		ResourceHandle m_materialHandle = 1;
	};
}