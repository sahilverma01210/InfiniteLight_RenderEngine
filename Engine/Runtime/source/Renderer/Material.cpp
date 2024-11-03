#include "Material.h"
#include "DynamicConstant.h"

namespace Renderer
{
	Material::Material(D3D12RHI& gfx, const aiMaterial& material, const std::filesystem::path& path) noexcept
		:
	modelPath(path.string())
	{
		UINT numSRVDescriptors = 0, srvDescriptorIndex = 0;

		std::string diffPath, specPath, normPath;
		std::wstring vertexShaderName, pixelShaderName;

		bool hasSpecularMap = false;
		bool hasNormalMap = false;
		bool hasDiffuseMap = false;

		const auto rootPath = path.parent_path().string() + "\\";

		vtxLayout.Append(VertexLayout::Position3D);
		vtxLayout.Append(VertexLayout::Normal);

		// phong technique
		{
			Technique phong{ "Phong" };
			Step step(0);
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
					vtxLayout.Append(VertexLayout::Texture2D);					
					numSRVDescriptors++;
				}
				else
				{
					pscLayout.Add<Float3>("materialColor");
				}
			}
			// specular
			{
				if (material.GetTexture(aiTextureType_SPECULAR, 0, &specFileName) == aiReturn_SUCCESS)
				{
					hasTexture = true;
					hasSpecularMap = true;
					shaderCode += L"Spc";
					vtxLayout.Append(VertexLayout::Texture2D);
					numSRVDescriptors++;
					pscLayout.Add<Bool>("useGlossAlpha");
					pscLayout.Add<Bool>("useSpecularMap");
				}
				pscLayout.Add<Float3>("specularColor");
				pscLayout.Add<Float>("specularWeight");
				pscLayout.Add<Float>("specularGloss");
			}
			// normal
			{
				if (material.GetTexture(aiTextureType_NORMALS, 0, &normFileName) == aiReturn_SUCCESS)
				{
					hasTexture = true;
					hasNormalMap = true;
					shaderCode += L"Nrm";
					vtxLayout.Append(VertexLayout::Texture2D);
					vtxLayout.Append(VertexLayout::Tangent);
					vtxLayout.Append(VertexLayout::Bitangent);
					numSRVDescriptors++;
					pscLayout.Add<Bool>("useNormalMap");
					pscLayout.Add<Float>("normalMapWeight");
				}
			}

			// Define the vertex input layout.
			std::vector<D3D12_INPUT_ELEMENT_DESC> vec = vtxLayout.GetD3DLayout();
			D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

			for (size_t i = 0; i < vec.size(); ++i) {
				inputElementDescs[i] = vec[i];
			}

			// common
			{
				// Add Pipeline State Obejct
				{
					ID3DBlob* vertexShader;
					ID3DBlob* pixelShader;

					// Compile Shaders.
					D3DCompileFromFile(gfx.GetAssetFullPath((shaderCode + L"_VS.hlsl").c_str()).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &vertexShader, nullptr);
					D3DCompileFromFile(gfx.GetAssetFullPath((shaderCode + L"_PS.hlsl").c_str()).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &pixelShader, nullptr);

					PipelineDescription phongPipelineDesc{};
					phongPipelineDesc.vertexShader = vertexShader;
					phongPipelineDesc.pixelShader = pixelShader;
					phongPipelineDesc.inputElementDescs = inputElementDescs;
					phongPipelineDesc.numElements = vec.size();
					phongPipelineDesc.numConstants = 1;
					phongPipelineDesc.numConstantBufferViews = 3;
					phongPipelineDesc.numSRVDescriptors = numSRVDescriptors;
					phongPipelineDesc.backFaceCulling = !hasAlpha;
					phongPipelineDesc.depthStencilMode = Mode::Off;

					pipelineDesc.push_back(phongPipelineDesc);
				}

				{
					if (hasTexture)
					{
						std::shared_ptr<ShaderResourceView> srvBindablePtr = std::make_shared<ShaderResourceView>(gfx, 4, numSRVDescriptors);

						if (hasDiffuseMap)
						{
							diffPath = rootPath + diffFileName.C_Str();
							auto tex = TextureBuffer::Resolve(gfx, std::wstring(diffPath.begin(), diffPath.end()).c_str());
							if (tex->HasAlpha())
							{
								hasAlpha = true;
								shaderCode += L"Msk";
							}
							step.AddBindable(std::move(tex));
							srvBindablePtr->AddResource(gfx, srvDescriptorIndex, dynamic_cast<TextureBuffer*>(step.GetBindables()[srvDescriptorIndex].get())->GetBuffer());
							srvDescriptorIndex++;
						}

						if (hasSpecularMap)
						{
							specPath = rootPath + specFileName.C_Str();
							auto tex = TextureBuffer::Resolve(gfx, std::wstring(specPath.begin(), specPath.end()).c_str());
							hasGlossAlpha = tex->HasAlpha();
							step.AddBindable(std::move(tex));
							srvBindablePtr->AddResource(gfx, srvDescriptorIndex, dynamic_cast<TextureBuffer*>(step.GetBindables()[srvDescriptorIndex].get())->GetBuffer());
							srvDescriptorIndex++;
						}

						if (hasNormalMap)
						{
							normPath = rootPath + normFileName.C_Str();
							auto tex = TextureBuffer::Resolve(gfx, std::wstring(normPath.begin(), normPath.end()).c_str());
							step.AddBindable(std::move(tex));
							srvBindablePtr->AddResource(gfx, srvDescriptorIndex, dynamic_cast<TextureBuffer*>(step.GetBindables()[srvDescriptorIndex].get())->GetBuffer());
							srvDescriptorIndex++;
						}

						step.AddBindable(std::move(srvBindablePtr));
					}
				}

				step.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));

				// PS material params (cbuf)
				Buffer buf{ std::move(pscLayout) };
				if (auto r = buf["materialColor"]; r.Exists())
				{
					aiColor3D color = { 0.45f,0.45f,0.85f };
					material.Get(AI_MATKEY_COLOR_DIFFUSE, color);
					r = reinterpret_cast<DirectX::XMFLOAT3&>(color);
				}
				buf["useGlossAlpha"].SetIfExists(hasGlossAlpha);
				buf["useSpecularMap"].SetIfExists(true);
				if (auto r = buf["specularColor"]; r.Exists())
				{
					aiColor3D color = { 0.18f,0.18f,0.18f };
					material.Get(AI_MATKEY_COLOR_SPECULAR, color);
					r = reinterpret_cast<DirectX::XMFLOAT3&>(color);
				}
				buf["specularWeight"].SetIfExists(1.0f);
				if (auto r = buf["specularGloss"]; r.Exists())
				{
					float gloss = 8.0f;
					material.Get(AI_MATKEY_SHININESS, gloss);
					r = gloss;
				}
				buf["useNormalMap"].SetIfExists(true);
				buf["normalMapWeight"].SetIfExists(1.0f);
				step.AddBindable(std::make_shared<ConstantBuffer>(gfx, 2, buf));
			}
			phong.AddStep(std::move(step));
			techniques.push_back(std::move(phong));
		}

		// outline technique
		{
			Technique outline("Outline", false);
			{
				Step mask(1);
		
				// Define the vertex input layout.
				std::vector<D3D12_INPUT_ELEMENT_DESC> vec = vtxLayout.GetD3DLayout();
				D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];
		
				for (size_t i = 0; i < vec.size(); ++i) {
					inputElementDescs[i] = vec[i];
				}
		
				// Add Pipeline State Obejct
				{
					ID3DBlob* vertexShader;
		
					// Compile Shaders.
					D3DCompileFromFile(gfx.GetAssetFullPath(L"Solid_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &vertexShader, nullptr);

					PipelineDescription maskPipelineDesc{};
					maskPipelineDesc.vertexShader = vertexShader;
					maskPipelineDesc.inputElementDescs = inputElementDescs;
					maskPipelineDesc.numElements = vec.size();
					maskPipelineDesc.numConstants = 1;
					maskPipelineDesc.numConstantBufferViews = 0;
					maskPipelineDesc.numSRVDescriptors = 0;
					maskPipelineDesc.backFaceCulling = false;
					maskPipelineDesc.depthStencilMode = Mode::Write;

					pipelineDesc.push_back(maskPipelineDesc);
				}
		
				mask.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));
				outline.AddStep(std::move(mask));
			}
			{
				Step draw(2);

				// Define the vertex input layout.
				std::vector<D3D12_INPUT_ELEMENT_DESC> vec = vtxLayout.GetD3DLayout();
				D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

				for (size_t i = 0; i < vec.size(); ++i) {
					inputElementDescs[i] = vec[i];
				}

				// Add Pipeline State Obejct
				{
					ID3DBlob* vertexShader;
					ID3DBlob* pixelShader;

					// Compile Shaders.
					D3DCompileFromFile(gfx.GetAssetFullPath(L"Solid_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &vertexShader, nullptr);
					D3DCompileFromFile(gfx.GetAssetFullPath(L"Solid_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &pixelShader, nullptr);

					PipelineDescription drawPipelineDesc{};
					drawPipelineDesc.vertexShader = vertexShader;
					drawPipelineDesc.pixelShader = pixelShader;
					drawPipelineDesc.inputElementDescs = inputElementDescs;
					drawPipelineDesc.numElements = vec.size();
					drawPipelineDesc.numConstants = 1;
					drawPipelineDesc.numConstantBufferViews = 1;
					drawPipelineDesc.numSRVDescriptors = 0;
					drawPipelineDesc.backFaceCulling = true;
					drawPipelineDesc.depthStencilMode = Mode::Off;

					pipelineDesc.push_back(drawPipelineDesc);
				}

				draw.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));

				{
					RawLayout lay;
					lay.Add<Float3>("materialColor");
					auto buf = Buffer(std::move(lay));
					buf["materialColor"] = DirectX::XMFLOAT3{ 1.0f,0.4f,0.4f };
					draw.AddBindable(std::make_shared<ConstantBuffer>(gfx, 1, buf));
				}

				outline.AddStep(std::move(draw));
			}
			techniques.push_back(std::move(outline));
		}
	}

	VertexRawBuffer Material::ExtractVertices(const aiMesh& mesh) const noexcept
	{
		return { vtxLayout,mesh };
	}
	std::vector<unsigned short> Material::ExtractIndices(const aiMesh& mesh) const noexcept
	{
		std::vector<unsigned short> indices;
		indices.reserve(mesh.mNumFaces * 3);
		for (unsigned int i = 0; i < mesh.mNumFaces; i++)
		{
			const auto& face = mesh.mFaces[i];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}
		return indices;
	}
	std::shared_ptr<VertexBuffer> Material::MakeVertexBindable(D3D12RHI& gfx, const aiMesh& mesh, float scale) const noexcept
	{
		auto vtc = ExtractVertices(mesh);
		if (scale != 1.0f)
		{
			for (auto i = 0u; i < vtc.Size(); i++)
			{
				DirectX::XMFLOAT3& pos = vtc[i].Attr<VertexLayout::ElementType::Position3D>();
				pos.x *= scale;
				pos.y *= scale;
				pos.z *= scale;
			}
		}
		return VertexBuffer::Resolve(gfx, MakeMeshTag(mesh), vtc.GetData(), UINT(vtc.SizeBytes()), (UINT)vtc.GetLayout().Size());
	}
	std::shared_ptr<IndexBuffer> Material::MakeIndexBindable(D3D12RHI& gfx, const aiMesh& mesh) const noexcept
	{
		auto indices = ExtractIndices(mesh);
		return IndexBuffer::Resolve(gfx, MakeMeshTag(mesh), indices.size() * sizeof(indices[0]), indices);
	}
	std::vector<PipelineDescription> Material::GetPipelineDesc() noexcept
	{
		return pipelineDesc;
	}
	std::string Material::MakeMeshTag(const aiMesh& mesh) const noexcept
	{
		return modelPath + "%" + mesh.mName.C_Str();
	}
	std::vector<Technique> Material::GetTechniques() noexcept
	{
		return techniques;
	}
}