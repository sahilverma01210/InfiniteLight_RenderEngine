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
			bool useGlossAlpha = false;
			bool useSpecularMap = false;
			bool useNormalMap = false;
		};

		__declspec(align(256u)) struct SolidCB
		{
			XMFLOAT3 materialColor;
		};

	public:
		ImportMaterial(D3D12RHI& gfx, const aiMaterial& material, const std::filesystem::path& path) noexcept(!IS_DEBUG)
		{
			m_vtxLayout.Append(VertexLayout::Position3D);
			m_vtxLayout.Append(VertexLayout::Normal);

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
						ID3DBlob* vertexShader;

						// Compile Shaders.
						D3DCompileFromFile(GetAssetFullPath(L"Shadow_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", 0, 0, &vertexShader, nullptr);

						PipelineDescription shadowMapPipelineDesc{};
						shadowMapPipelineDesc.numConstants = 1;
						shadowMapPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
						shadowMapPipelineDesc.shadowMapping = true;
						shadowMapPipelineDesc.numElements = vec.size();
						shadowMapPipelineDesc.inputElementDescs = inputElementDescs;
						shadowMapPipelineDesc.vertexShader = vertexShader;
						shadowMapPipelineDesc.depthUsage = DepthUsage::ShadowDepth;

						m_pipelineDesc["shadowMap"] = shadowMapPipelineDesc;
					}

					draw.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));
				}
				map.AddStep(std::move(draw));
			}
			m_techniques.push_back(std::move(map));

			// phong technique
			Technique phong{ "Phong",Channel::main };
			{
				// 1. Lambertian Step
				Step step("lambertian");
				{
					UINT numSRVDescriptors = 0;

					std::string diffPath, specPath, normPath;
					std::shared_ptr<TextureBuffer> diffTex, specTex, normTex;
					std::wstring vertexShaderName, pixelShaderName;

					bool hasSpecularMap = false;
					bool hasNormalMap = false;
					bool hasDiffuseMap = false;

					const auto rootPath = path.parent_path().string() + "\\";

					std::wstring shaderCode = L"Phong";
					aiString diffFileName, specFileName, normFileName;
					// common (pre)
					RawLayout pscLayout;
					bool hasAlpha = false;
					bool hasTexture = false;
					bool hasGlossAlpha = false;

					// diffuse
					{
						if (material.GetTexture(aiTextureType_DIFFUSE, 0, &diffFileName) == aiReturn_SUCCESS)
						{
							hasTexture = true;
							hasDiffuseMap = true;
							shaderCode += L"Dif";
							diffPath = rootPath + diffFileName.C_Str();
							diffTex = std::move(TextureBuffer::Resolve(gfx, std::wstring(diffPath.begin(), diffPath.end()).c_str()));
							if (diffTex->HasAlpha())
							{
								hasAlpha = true;
								shaderCode += L"Msk";
							}
							m_vtxLayout.Append(VertexLayout::Texture2D);
							numSRVDescriptors++;
						}
						else
						{
							aiColor3D color = { 0.45f,0.45f,0.85f };
							material.Get(AI_MATKEY_COLOR_DIFFUSE, color);
							m_phongData.materialColor = reinterpret_cast<XMFLOAT3&>(color);
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

						if (material.GetTexture(aiTextureType_SPECULAR, 0, &specFileName) == aiReturn_SUCCESS)
						{
							hasTexture = true;
							hasSpecularMap = true;
							shaderCode += L"Spc";
							specPath = rootPath + specFileName.C_Str();
							specTex = std::move(TextureBuffer::Resolve(gfx, std::wstring(specPath.begin(), specPath.end()).c_str()));
							hasGlossAlpha = specTex->HasAlpha();
							m_vtxLayout.Append(VertexLayout::Texture2D);
							numSRVDescriptors++;

							m_phongData.useGlossAlpha = hasGlossAlpha;
							m_phongData.useSpecularMap = true;
						}
					}
					// normal
					{
						if (material.GetTexture(aiTextureType_NORMALS, 0, &normFileName) == aiReturn_SUCCESS)
						{
							hasTexture = true;
							hasNormalMap = true;
							shaderCode += L"Nrm";
							normPath = rootPath + normFileName.C_Str();
							normTex = std::move(TextureBuffer::Resolve(gfx, std::wstring(normPath.begin(), normPath.end()).c_str()));
							m_vtxLayout.Append(VertexLayout::Texture2D);
							m_vtxLayout.Append(VertexLayout::Tangent);
							m_vtxLayout.Append(VertexLayout::Bitangent);
							numSRVDescriptors++;

							m_phongData.useNormalMap = true;
							m_phongData.normalMapWeight = 1.0f;
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
						ID3DBlob* vertexShader;
						ID3DBlob* pixelShader;

						// Compile Shaders.
						D3DCompileFromFile(GetAssetFullPath((shaderCode + L"_VS.hlsl").c_str()).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", 0, 0, &vertexShader, nullptr);
						D3DCompileFromFile(GetAssetFullPath((shaderCode + L"_PS.hlsl").c_str()).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", 0, 0, &pixelShader, nullptr);

						numSRVDescriptors++; // One extra for Shadow Texture.

						CD3DX12_STATIC_SAMPLER_DESC* staticSamplers = new CD3DX12_STATIC_SAMPLER_DESC[2];

						CD3DX12_STATIC_SAMPLER_DESC sampler1{ 0 };
						sampler1.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
						sampler1.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
						sampler1.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
						sampler1.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
						sampler1.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
						staticSamplers[0] = sampler1;

						CD3DX12_STATIC_SAMPLER_DESC sampler2{ 1 };
						sampler2.Filter = D3D12_FILTER_ANISOTROPIC;
						sampler2.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
						sampler2.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
						sampler2.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
						sampler2.MipLODBias = 0.0f;
						sampler2.MinLOD = 0.0f;
						sampler2.MaxLOD = D3D12_FLOAT32_MAX;
						staticSamplers[1] = sampler2;

						PipelineDescription phongPipelineDesc{};
						phongPipelineDesc.numConstants = 1;
						phongPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
						phongPipelineDesc.numConstantBufferViews = 3;
						phongPipelineDesc.numShaderResourceViews = numSRVDescriptors;
						phongPipelineDesc.numStaticSamplers = 2; // One extra Sampler for Shadow Texture.
						phongPipelineDesc.staticSamplers = staticSamplers;
						phongPipelineDesc.backFaceCulling = !hasAlpha;
						phongPipelineDesc.numElements = vec.size();
						phongPipelineDesc.inputElementDescs = inputElementDescs;
						phongPipelineDesc.vertexShader = vertexShader;
						phongPipelineDesc.pixelShader = pixelShader;

						m_pipelineDesc["lambertian"] = phongPipelineDesc;
					}

					step.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));

					std::shared_ptr<DescriptorTable> descriptorTable = std::make_shared<DescriptorTable>(gfx, 1, numSRVDescriptors + 3);

					descriptorTable->AddConstantBufferView(gfx, m_lightShadowBindable->GetBuffer());
					step.AddBindable(m_lightShadowBindable);
					descriptorTable->AddConstantBufferView(gfx, m_lightBindable->GetBuffer());
					step.AddBindable(m_lightBindable);

					std::shared_ptr<ConstantBuffer> constBuffer = std::make_shared<ConstantBuffer>(gfx, sizeof(m_phongData), &m_phongData);

					descriptorTable->AddConstantBufferView(gfx, constBuffer->GetBuffer());
					step.AddBindable(constBuffer);

					// Add Textures
					if (hasTexture)
					{
						// Link Shadow Texture;
						descriptorTable->AddShaderResourceView(gfx, gfx.GetDepthBuffer(), false, true);

						if (hasDiffuseMap) descriptorTable->AddShaderResourceView(gfx, diffTex->GetBuffer());
						if (hasSpecularMap) descriptorTable->AddShaderResourceView(gfx, specTex->GetBuffer());
						if (hasNormalMap) descriptorTable->AddShaderResourceView(gfx, normTex->GetBuffer());
					}

					step.AddBindable(std::move(descriptorTable));
				}
				phong.AddStep(std::move(step));
			}
			m_techniques.push_back(std::move(phong));

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
						ID3DBlob* vertexShader;

						// Compile Shaders.
						D3DCompileFromFile(GetAssetFullPath(L"Solid_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", 0, 0, &vertexShader, nullptr);

						PipelineDescription maskPipelineDesc{};
						maskPipelineDesc.numConstants = 1;
						maskPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
						maskPipelineDesc.depthStencilMode = Mode::Write;
						maskPipelineDesc.numElements = vec.size();
						maskPipelineDesc.inputElementDescs = inputElementDescs;
						maskPipelineDesc.vertexShader = vertexShader;

						m_pipelineDesc["outlineMask"] = maskPipelineDesc;
					}

					mask.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));
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
						ID3DBlob* vertexShader;
						ID3DBlob* pixelShader;

						// Compile Shaders.
						D3DCompileFromFile(GetAssetFullPath(L"Solid_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", 0, 0, &vertexShader, nullptr);
						D3DCompileFromFile(GetAssetFullPath(L"Solid_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", 0, 0, &pixelShader, nullptr);

						PipelineDescription drawPipelineDesc{};
						drawPipelineDesc.vertexShader = vertexShader;
						drawPipelineDesc.pixelShader = pixelShader;
						drawPipelineDesc.inputElementDescs = inputElementDescs;
						drawPipelineDesc.numElements = vec.size();
						drawPipelineDesc.numConstants = 1;
						drawPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
						drawPipelineDesc.numConstantBufferViews = 1;
						drawPipelineDesc.backFaceCulling = true;
						drawPipelineDesc.depthUsage = DepthUsage::None;

						m_pipelineDesc["outlineDraw"] = drawPipelineDesc;
					}

					draw.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));

					std::shared_ptr<DescriptorTable> descriptorTable = std::move(std::make_unique<DescriptorTable>(gfx, 1, 1));

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