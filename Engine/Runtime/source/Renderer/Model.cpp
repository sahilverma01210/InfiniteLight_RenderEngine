#include "Model.h"

using namespace Common;

namespace Renderer
{
	// ModelException Definitions.

	ModelException::ModelException(int line, const char* file, std::string note) noexcept
		:
		ILException(line, file),
		note(std::move(note))
	{}

	const char* ModelException::what() const noexcept
	{
		std::ostringstream oss;
		oss << ILException::what() << std::endl
			<< "[Note] " << GetNote();
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}

	const char* ModelException::GetType() const noexcept
	{
		return "IL Model Exception";
	}

	const std::string& ModelException::GetNote() const noexcept
	{
		return note;
	}

	// Model.

	class ModelWindow // pImpl idiom, only defined in this .cpp
	{
	public:
		void Show(const char* windowName, const Node& root) noexcept
		{
			// window name defaults to "Model"
			windowName = windowName ? windowName : "Model";
			// need an ints to track node indices and selected node
			int nodeIndexTracker = 0;

			if (ImGui::Begin(windowName))
			{
				ImGui::Columns(2, nullptr, true);
				root.ShowTree(pSelectedNode);

				ImGui::NextColumn();
				if (pSelectedNode != nullptr)
				{
					const auto id = pSelectedNode->GetId();
					auto i = transforms.find(id);
					if (i == transforms.end())
					{
						const auto& applied = pSelectedNode->GetAppliedTransform();
						const auto angles = ExtractEulerAngles(applied);
						const auto translation = ExtractTranslation(applied);
						TransformParameters tp;
						tp.roll = angles.z;
						tp.pitch = angles.x;
						tp.yaw = angles.y;
						tp.x = translation.x;
						tp.y = translation.y;
						tp.z = translation.z;
						std::tie(i, std::ignore) = transforms.insert({ id,tp });
					}
					auto& transform = i->second;
					ImGui::Text("Orientation");
					ImGui::SliderAngle("Roll", &transform.roll, -180.0f, 180.0f);
					ImGui::SliderAngle("Pitch", &transform.pitch, -180.0f, 180.0f);
					ImGui::SliderAngle("Yaw", &transform.yaw, -180.0f, 180.0f);
					ImGui::Text("Position");
					ImGui::SliderFloat("X", &transform.x, -20.0f, 20.0f);
					ImGui::SliderFloat("Y", &transform.y, -20.0f, 20.0f);
					ImGui::SliderFloat("Z", &transform.z, -20.0f, 20.0f);
				}
			}
			ImGui::End();
		}
		XMMATRIX GetTransform() const noexcept
		{
			assert(pSelectedNode != nullptr);
			const auto& transform = transforms.at(pSelectedNode->GetId());
			return
				XMMatrixRotationRollPitchYaw(transform.roll, transform.pitch, transform.yaw) *
				XMMatrixTranslation(transform.x, transform.y, transform.z);
		}
		Node* GetSelectedNode() const noexcept
		{
			return pSelectedNode;
		}
	private:
		Node* pSelectedNode;
		struct TransformParameters
		{
			float roll = 0.0f;
			float pitch = 0.0f;
			float yaw = 0.0f;
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
		};
		std::unordered_map<int, TransformParameters> transforms;
	};

	// Model Definitions.

	Model::Model(D3D12RHI& gfx, const std::string& pathString, float fscale)
		:
		pWindow(std::make_unique<ModelWindow>())
	{
		Assimp::Importer imp;
		const auto pScene = imp.ReadFile(pathString.c_str(),
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace
		);

		if (pScene == nullptr)
		{
			throw ModelException(__LINE__, __FILE__, imp.GetErrorString());
		}

		for (size_t i = 0; i < pScene->mNumMeshes; i++)
		{
			meshPtrs.push_back(ParseMesh(gfx, *pScene->mMeshes[i], pScene->mMaterials, pathString, fscale));
		}

		int nextId = 0;
		pRoot = ParseNode(nextId, *pScene->mRootNode);
	}

	void Model::Draw(D3D12RHI& gfx) const noexcept
	{
		if (auto node = pWindow->GetSelectedNode())
		{
			node->SetAppliedTransform(pWindow->GetTransform());
		}
		pRoot->Draw(gfx, XMMatrixIdentity());
	}

	void Model::ShowWindow(const char* windowName) noexcept
	{
		pWindow->Show(windowName, *pRoot);
	}

	void Model::SetRootTransform(DirectX::FXMMATRIX tf) noexcept
	{
		pRoot->SetAppliedTransform(tf);
	}

	std::unique_ptr<Mesh> Model::ParseMesh(D3D12RHI& gfx, const aiMesh& mesh, const aiMaterial* const* pMaterials, const std::filesystem::path& path, float fscale)
	{
		using namespace std::string_literals;

		const auto rootPath = path.parent_path().string() + "\\";

		UINT numSRVDescriptors = 0, srvDescriptorIndex = 0;

		std::unique_ptr<Bindable> rootSignBindablePtr;
		std::unique_ptr<Bindable> psoBindablePtr;
		std::unique_ptr<Bindable> srvBindablePtr;
		std::vector<std::shared_ptr<Bindable>> bindablePtrs;

		std::string diffPath, specPath, normPath;
		std::wstring vertexShaderName, pixelShaderName;

		bool hasAlphaGloss = false;
		bool hasAlphaDiffuse = false;
		bool hasSpecularMap = false;
		bool hasNormalMap = false;
		bool hasDiffuseMap = false;

		float shininess = 2.0f;
		XMFLOAT4 specularColor = { 0.18f,0.18f,0.18f,1.0f };
		XMFLOAT4 diffuseColor = { 0.45f,0.45f,0.85f,1.0f };

		const float scale = fscale;

		VertexRawBuffer vbuf;

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

		if (mesh.mMaterialIndex >= 0)
		{
			using namespace std::string_literals;
			auto& material = *pMaterials[mesh.mMaterialIndex];

			aiString texFileName;

			if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName) == aiReturn_SUCCESS)
			{
				diffPath = rootPath + texFileName.C_Str();
				auto tex = TextureBuffer::Resolve(gfx, std::wstring(diffPath.begin(), diffPath.end()).c_str());
				hasAlphaDiffuse = tex->HasAlpha();
				bindablePtrs.push_back(std::move(tex));
				hasDiffuseMap = true;
				numSRVDescriptors++;
			}
			else
			{
				material.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>(diffuseColor));
			}

			if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
			{
				specPath = rootPath + texFileName.C_Str();
				auto tex = TextureBuffer::Resolve(gfx, std::wstring(specPath.begin(), specPath.end()).c_str());
				hasAlphaGloss = tex->HasAlpha();
				bindablePtrs.push_back(std::move(tex));
				hasSpecularMap = true;
				numSRVDescriptors++;
			}
			else
			{
				material.Get(AI_MATKEY_COLOR_SPECULAR, reinterpret_cast<aiColor3D&>(specularColor));
			}
			if (!hasAlphaGloss)
			{
				material.Get(AI_MATKEY_SHININESS, shininess);
			}

			if (material.GetTexture(aiTextureType_NORMALS, 0, &texFileName) == aiReturn_SUCCESS)
			{
				normPath = rootPath + texFileName.C_Str();
				auto tex = TextureBuffer::Resolve(gfx, std::wstring(normPath.begin(), normPath.end()).c_str());
				hasAlphaGloss = tex->HasAlpha();
				bindablePtrs.push_back(std::move(tex));
				hasNormalMap = true;
				numSRVDescriptors++;
			}
		}

		if (hasDiffuseMap && hasNormalMap && hasSpecularMap)
		{
			vertexShaderName = L"PhongVSNormalMap.hlsl";
			pixelShaderName = hasAlphaDiffuse ? L"PhongPSSpecNormMask.hlsl" : L"PhongPSSpecNormalMap.hlsl";

			vbuf.SetLayout(VertexLayout{}
				.Append(VertexLayout::Position3D)
				.Append(VertexLayout::Normal)
				.Append(VertexLayout::Tangent)
				.Append(VertexLayout::Bitangent)
				.Append(VertexLayout::Texture2D));

			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
			{
				vbuf.EmplaceBack(
					XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
					*reinterpret_cast<XMFLOAT3*>(&mesh.mNormals[i]),
					*reinterpret_cast<XMFLOAT3*>(&mesh.mTangents[i]),
					*reinterpret_cast<XMFLOAT3*>(&mesh.mBitangents[i]),
					*reinterpret_cast<XMFLOAT2*>(&mesh.mTextureCoords[0][i])
				);
			}
		}
		else if (hasDiffuseMap && hasNormalMap)
		{
			vertexShaderName = L"PhongVSNormalMap.hlsl";
			pixelShaderName = L"PhongPSNormalMap.hlsl";

			vbuf.SetLayout(VertexLayout{}
				.Append(VertexLayout::Position3D)
				.Append(VertexLayout::Normal)
				.Append(VertexLayout::Tangent)
				.Append(VertexLayout::Bitangent)
				.Append(VertexLayout::Texture2D));

			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
			{
				vbuf.EmplaceBack(
					XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
					*reinterpret_cast<XMFLOAT3*>(&mesh.mNormals[i]),
					*reinterpret_cast<XMFLOAT3*>(&mesh.mTangents[i]),
					*reinterpret_cast<XMFLOAT3*>(&mesh.mBitangents[i]),
					*reinterpret_cast<XMFLOAT2*>(&mesh.mTextureCoords[0][i])
				);
			}
		}
		else if (hasDiffuseMap && !hasNormalMap && hasSpecularMap)
		{
			vertexShaderName = L"PhongVS.hlsl";
			pixelShaderName = L"PhongPSSpec.hlsl";

			vbuf.SetLayout(VertexLayout{}
				.Append(VertexLayout::Position3D)
				.Append(VertexLayout::Normal)
				.Append(VertexLayout::Texture2D));

			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
			{
				vbuf.EmplaceBack(
					XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
					*reinterpret_cast<XMFLOAT3*>(&mesh.mNormals[i]),
					*reinterpret_cast<XMFLOAT2*>(&mesh.mTextureCoords[0][i])
				);
			}
		}
		else if (hasDiffuseMap)
		{
			vertexShaderName = L"PhongVS.hlsl";
			pixelShaderName = L"PhongPS.hlsl";

			vbuf.SetLayout(VertexLayout{}
				.Append(VertexLayout::Position3D)
				.Append(VertexLayout::Normal)
				.Append(VertexLayout::Texture2D));

			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
			{
				vbuf.EmplaceBack(
					XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
					*reinterpret_cast<XMFLOAT3*>(&mesh.mNormals[i]),
					*reinterpret_cast<XMFLOAT2*>(&mesh.mTextureCoords[0][i])
				);
			}
		}
		else if (!hasDiffuseMap && !hasNormalMap && !hasSpecularMap)
		{
			vertexShaderName = L"PhongVSNotex.hlsl";
			pixelShaderName = L"PhongPSNotex.hlsl";

			vbuf.SetLayout(VertexLayout{}
				.Append(VertexLayout::Position3D)
				.Append(VertexLayout::Normal));

			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
			{
				vbuf.EmplaceBack(
					XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
					*reinterpret_cast<XMFLOAT3*>(&mesh.mNormals[i])
				);
			}
		}
		else
		{
			throw std::runtime_error("terrible combination of textures in material smh");
		}

		// Create Pipeline State Obejct & Shader Resource View
		{
			ID3DBlob* vertexShader;
			ID3DBlob* pixelShader;

			// Compile Shaders.
			D3DCompileFromFile(gfx.GetAssetFullPath(vertexShaderName.c_str()).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &vertexShader, nullptr);
			D3DCompileFromFile(gfx.GetAssetFullPath(pixelShaderName.c_str()).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &pixelShader, nullptr);

			// Define the vertex input layout.
			std::vector<D3D12_INPUT_ELEMENT_DESC> vec = vbuf.GetLayout().GetD3DLayout();
			D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

			for (size_t i = 0; i < vec.size(); ++i) {
				inputElementDescs[i] = vec[i];
			}

			PipelineDescription pipelineDesc{ *vertexShader, *pixelShader, *inputElementDescs, vec.size(), 1, 2, numSRVDescriptors, hasAlphaDiffuse };

			rootSignBindablePtr = std::make_unique<RootSignature>(gfx, pipelineDesc);
			psoBindablePtr = std::make_unique<PipelineState>(gfx, pipelineDesc);
			if (pipelineDesc.numSRVDescriptors > 0) srvBindablePtr = std::make_unique<ShaderResourceView>(gfx, 3, numSRVDescriptors);
		}

		// Add Other Bindables
		{
			bindablePtrs.push_back(std::make_shared<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
			bindablePtrs.push_back(std::make_shared<VertexBuffer>(gfx, vbuf.GetData(), UINT(vbuf.SizeBytes()), (UINT)vbuf.GetLayout().Size()));
			bindablePtrs.push_back(std::make_shared<IndexBuffer>(gfx, indices.size() * sizeof(indices[0]), indices));

			if (srvBindablePtr != nullptr)
			{
				auto srv = dynamic_cast<ShaderResourceView*>(srvBindablePtr.get());

				if (hasDiffuseMap)
				{					
					srv->AddResource(gfx, srvDescriptorIndex, dynamic_cast<TextureBuffer*>(bindablePtrs[srvDescriptorIndex].get())->GetBuffer());					
					srvDescriptorIndex++;
				}

				if (hasSpecularMap)
				{
					srv->AddResource(gfx, srvDescriptorIndex, dynamic_cast<TextureBuffer*>(bindablePtrs[srvDescriptorIndex].get())->GetBuffer());					
					srvDescriptorIndex++;
				}

				if (hasNormalMap)
				{
					srv->AddResource(gfx, srvDescriptorIndex, dynamic_cast<TextureBuffer*>(bindablePtrs[srvDescriptorIndex].get())->GetBuffer());					
					srvDescriptorIndex++;
				}
			}			
		}

		if (hasDiffuseMap && hasNormalMap && hasSpecularMap)
		{
			Node::PSMaterialConstantFullmonte pmc;
			pmc.specularPower = shininess;
			pmc.hasGlossMap = hasAlphaGloss ? TRUE : FALSE;
			bindablePtrs.push_back(std::make_shared<ConstantBuffer>(gfx, 2, sizeof(pmc), &pmc));
		}
		else if (hasDiffuseMap && hasNormalMap)
		{
			struct PSMaterialConstantDiffnorm
			{
				float specularIntensity;
				float specularPower;
				BOOL  normalMapEnabled = TRUE;
				float padding[1];
			} pmc;
			pmc.specularPower = shininess;
			pmc.specularIntensity = (specularColor.x + specularColor.y + specularColor.z) / 3.0f;
			bindablePtrs.push_back(std::make_shared<ConstantBuffer>(gfx, 2, sizeof(pmc), &pmc));
		}
		else if (hasDiffuseMap && !hasNormalMap && hasSpecularMap)
		{
			struct PSMaterialConstantDiffuseSpec
			{
				float specularPowerConst;
				BOOL hasGloss;
				float specularMapWeight;
				float padding;
			} pmc;
			pmc.specularPowerConst = shininess;
			pmc.hasGloss = hasAlphaGloss ? TRUE : FALSE;
			pmc.specularMapWeight = 1.0f;
			bindablePtrs.push_back(std::make_shared<ConstantBuffer>(gfx, 2, sizeof(pmc), &pmc));
		}
		else if (hasDiffuseMap)
		{
			struct PSMaterialConstantDiffuse
			{
				float specularIntensity;
				float specularPower;
				float padding[2];
			} pmc;
			pmc.specularPower = shininess;
			pmc.specularIntensity = (specularColor.x + specularColor.y + specularColor.z) / 3.0f;
			bindablePtrs.push_back(std::make_shared<ConstantBuffer>(gfx, 2, sizeof(pmc), &pmc));
		}
		else if (!hasDiffuseMap && !hasNormalMap && !hasSpecularMap)
		{
			Node::PSMaterialConstantNotex pmc;
			pmc.specularPower = shininess;
			pmc.specularColor = specularColor;
			pmc.materialColor = diffuseColor;
			bindablePtrs.push_back(std::make_shared<ConstantBuffer>(gfx, 2, sizeof(pmc), &pmc));
		}

		bindablePtrs.push_back(std::make_shared<TransformBuffer>(gfx, 0));
		

		std::unique_ptr<Mesh> temp_mesh = std::make_unique<Mesh>(gfx, std::move(rootSignBindablePtr), std::move(psoBindablePtr), std::move(srvBindablePtr), std::move(bindablePtrs));
		temp_mesh->SetNumIndices(indices.size() * sizeof(indices[0]));

		return temp_mesh;
	}

	std::unique_ptr<Node> Model::ParseNode(int& nextId, const aiNode& node) noexcept
	{
		namespace dx = DirectX;
		const auto transform = dx::XMMatrixTranspose(dx::XMLoadFloat4x4(
			reinterpret_cast<const dx::XMFLOAT4X4*>(&node.mTransformation)
		));

		std::vector<Mesh*> curMeshPtrs;
		curMeshPtrs.reserve(node.mNumMeshes);
		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			const auto meshIdx = node.mMeshes[i];
			curMeshPtrs.push_back(meshPtrs.at(meshIdx).get());
		}

		auto pNode = std::make_unique<Node>(nextId++, node.mName.C_Str(), std::move(curMeshPtrs), transform);
		for (size_t i = 0; i < node.mNumChildren; i++)
		{
			pNode->AddChild(ParseNode(nextId, *node.mChildren[i]));
		}

		return pNode;
	}

	Model::~Model() noexcept
	{
	}
}