#pragma once
#include "ILMaterial.h"

struct aiMaterial;

namespace Renderer
{
	class ImportMaterial : public ILMaterial
	{
		friend class PointLight;

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
			bool useSpecularMap = false;
			bool useNormalMap = false;
		};

		__declspec(align(256u)) struct SolidCB
		{
			XMFLOAT3 materialColor;
		};

	public:
		ImportMaterial() = default;
		ImportMaterial(D3D12RHI& gfx, const aiMaterial& material, const std::filesystem::path& path) noexcept(!IS_DEBUG)
		{
			m_vtxLayout.Append(VertexLayout::Position3D);
			m_vtxLayout.Append(VertexLayout::Normal);
			m_vtxLayout.Append(VertexLayout::Texture2D);
			m_vtxLayout.Append(VertexLayout::Tangent);
			m_vtxLayout.Append(VertexLayout::Bitangent);

			// shadow map technique
			Technique map{ "ShadowMap",Channel::shadow,true };
			{
				// 1. Shadow Map Step
				Step draw("shadowMap");
				{
					// Define the vertex input layout.
					std::vector<D3D12_INPUT_ELEMENT_DESC> vec = m_vtxLayout.GetD3DLayout();
					D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

					for (size_t i = 0; i < vec.size(); ++i) {
						inputElementDescs[i] = vec[i];
					}

					// Add Pipeline State Obejct
					{
						PipelineDescription shadowMapPipelineDesc{};
						shadowMapPipelineDesc.numConstants = 1;
						shadowMapPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
						shadowMapPipelineDesc.shadowMapping = true;
						shadowMapPipelineDesc.numElements = vec.size();
						shadowMapPipelineDesc.inputElementDescs = inputElementDescs;
						shadowMapPipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, GetAssetFullPath(L"Shadow_VS.hlsl") };
						shadowMapPipelineDesc.depthUsage = DepthUsage::ShadowDepth;

						m_pipelineDesc["shadowMap"] = shadowMapPipelineDesc;
					}
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

					std::shared_ptr<TextureBuffer> diffTex, specTex, normTex;

					const auto rootPath = path.parent_path().string() + "\\";

					// common (pre)
					RawLayout pscLayout;
					std::vector<SHADER_MACRO> macros;

					// diffuse
					{
						if (aiString diffFileName; m_phongData.useDiffuseMap = material.GetTexture(aiTextureType_DIFFUSE, 0, &diffFileName) == aiReturn_SUCCESS)
						{							
							std::string diffPath = rootPath + diffFileName.C_Str();
							diffTex = std::move(TextureBuffer::Resolve(gfx, diffPath));

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
						if (aiString normFileName; m_phongData.useNormalMap = material.GetTexture(aiTextureType_NORMALS, 0, &normFileName) == aiReturn_SUCCESS)
						{
							std::string normPath = rootPath + normFileName.C_Str();
							normTex = std::move(TextureBuffer::Resolve(gfx, normPath));

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

						if (aiString specFileName; m_phongData.useSpecularMap = material.GetTexture(aiTextureType_SPECULAR, 0, &specFileName) == aiReturn_SUCCESS)
						{
							std::string specPath = rootPath + specFileName.C_Str();
							specTex = std::move(TextureBuffer::Resolve(gfx, specPath));

							macros.push_back({ L"USE_SPECULAR_MAP", L"1" });
							numSRVDescriptors++;
						}
					}

					// Define the vertex input layout.
					std::vector<D3D12_INPUT_ELEMENT_DESC> vec = m_vtxLayout.GetD3DLayout();
					D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

					for (size_t i = 0; i < vec.size(); ++i) {
						inputElementDescs[i] = vec[i];
					}

					// Add Pipeline State Obejct
					{
						numSRVDescriptors++; // One extra for Shadow Texture.

						PipelineDescription phongPipelineDesc{};
						phongPipelineDesc.numConstants = 1;
						phongPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
						phongPipelineDesc.numConstantBufferViews = 3;
						phongPipelineDesc.numShaderResourceViews = numSRVDescriptors;
						phongPipelineDesc.numSamplers = 2;
						phongPipelineDesc.backFaceCulling = !m_phongData.useDiffuseAlpha;
						phongPipelineDesc.numElements = vec.size();
						phongPipelineDesc.inputElementDescs = inputElementDescs;
						phongPipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, GetAssetFullPath(L"Phong_VS.hlsl") };
						phongPipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, GetAssetFullPath(L"Phong_PS.hlsl") , macros };

						m_pipelineDesc["phong_shading"] = phongPipelineDesc;
					}

					DescriptorTable::TableParams params;
					params.resourceParameterIndex = 1;
					params.samplerParameterIndex = 2;
					params.numCbvSrvUavDescriptors = numSRVDescriptors + 3;
					params.numSamplerDescriptors = 2;

					std::shared_ptr<DescriptorTable> descriptorTable = std::make_shared<DescriptorTable>(gfx, params);

					// Add Constants
					{
						if (m_phongData.useDiffuseMap) m_phongData.useDiffuseAlpha = diffTex->HasAlpha();
						if (m_phongData.useSpecularMap) m_phongData.useGlossAlpha = specTex->HasAlpha();
						if (m_phongData.useNormalMap) m_phongData.normalMapWeight = 1.0f;

						std::shared_ptr<ConstantBuffer> constBuffer = std::make_shared<ConstantBuffer>(gfx, sizeof(m_phongData), &m_phongData);

						descriptorTable->AddConstantBufferView(gfx, m_lightShadowBindable->GetBuffer());
						step.AddBindable(m_lightShadowBindable);
						descriptorTable->AddConstantBufferView(gfx, m_lightBindable->GetBuffer());
						step.AddBindable(m_lightBindable);
						descriptorTable->AddConstantBufferView(gfx, constBuffer->GetBuffer());
						step.AddBindable(constBuffer);
					}

					// Add Textures
					{
						// Link Shadow Texture;
						descriptorTable->AddShaderResourceView(gfx, gfx.GetDepthBuffer(), false, true);

						if (m_phongData.useDiffuseMap) descriptorTable->AddShaderResourceView(gfx, diffTex->GetBuffer());
						if (m_phongData.useNormalMap) descriptorTable->AddShaderResourceView(gfx, normTex->GetBuffer());
						if (m_phongData.useSpecularMap) descriptorTable->AddShaderResourceView(gfx, specTex->GetBuffer());
					}

					// Add Samplers
					{
						D3D12_SAMPLER_DESC shadowSampler{};
						shadowSampler.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
						shadowSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
						shadowSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
						shadowSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
						shadowSampler.BorderColor[0] = 1.0f;
						shadowSampler.BorderColor[1] = 1.0f;
						shadowSampler.BorderColor[2] = 1.0f;
						shadowSampler.BorderColor[3] = 1.0f;
						shadowSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
						descriptorTable->AddSampler(gfx, &shadowSampler);

						D3D12_SAMPLER_DESC phongSampler{};
						phongSampler.Filter = D3D12_FILTER_ANISOTROPIC;
						phongSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
						phongSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
						phongSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
						phongSampler.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
						phongSampler.MipLODBias = 0.0f;
						phongSampler.MinLOD = 0.0f;
						phongSampler.MaxLOD = D3D12_FLOAT32_MAX;
						descriptorTable->AddSampler(gfx, &phongSampler);						
					}

					step.AddBindable(std::move(descriptorTable));
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
						// Define the vertex input layout.
						std::vector<D3D12_INPUT_ELEMENT_DESC> vec = m_vtxLayout.GetD3DLayout();
						D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

						for (size_t i = 0; i < vec.size(); ++i) {
							inputElementDescs[i] = vec[i];
						}

						// Add Pipeline State Obejct
						{
							PipelineDescription maskPipelineDesc{};
							maskPipelineDesc.numConstants = 1;
							maskPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
							maskPipelineDesc.depthStencilMode = Mode::Write;
							maskPipelineDesc.numElements = vec.size();
							maskPipelineDesc.inputElementDescs = inputElementDescs;
							maskPipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, GetAssetFullPath(L"Solid_VS.hlsl") };

							m_pipelineDesc["outlineMask"] = maskPipelineDesc;
						}
					}
					outline.AddStep(std::move(mask));

					// 2. Outline Draw Step
					Step draw("outlineDraw");
					{
						// Define the vertex input layout.
						std::vector<D3D12_INPUT_ELEMENT_DESC> vec = m_vtxLayout.GetD3DLayout();
						D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

						for (size_t i = 0; i < vec.size(); ++i) {
							inputElementDescs[i] = vec[i];
						}

						// Add Pipeline State Obejct
						{
							PipelineDescription drawPipelineDesc{};
							drawPipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, GetAssetFullPath(L"Solid_VS.hlsl") };
							drawPipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, GetAssetFullPath(L"Solid_PS.hlsl") };
							drawPipelineDesc.inputElementDescs = inputElementDescs;
							drawPipelineDesc.numElements = vec.size();
							drawPipelineDesc.numConstants = 1;
							drawPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
							drawPipelineDesc.numConstantBufferViews = 1;
							drawPipelineDesc.backFaceCulling = true;
							drawPipelineDesc.depthUsage = DepthUsage::None;

							m_pipelineDesc["outlineDraw"] = drawPipelineDesc;
						}

						DescriptorTable::TableParams params;
						params.resourceParameterIndex = 1;
						params.numCbvSrvUavDescriptors = 1;

						std::shared_ptr<DescriptorTable> descriptorTable = std::move(std::make_unique<DescriptorTable>(gfx, params));

						SolidCB data = { XMFLOAT3{ 1.0f,0.4f,0.4f } };
						std::shared_ptr<ConstantBuffer> constBuffer = std::make_shared<ConstantBuffer>(gfx, sizeof(data), static_cast<const void*>(&data));
						descriptorTable->AddConstantBufferView(gfx, constBuffer->GetBuffer());

						draw.AddBindable(constBuffer);
						draw.AddBindable(descriptorTable);
					}
					outline.AddStep(std::move(draw));
				}
				m_techniques.push_back(std::move(outline));
			}
		}
		VertexLayout& GetVertexLayout()
		{
			return m_vtxLayout;
		}

	private:
		PhongCB m_phongData;
		VertexLayout m_vtxLayout;
		static std::shared_ptr<ConstantBuffer> m_lightBindable;
		static std::shared_ptr<ConstantBuffer> m_lightShadowBindable;
	};
}