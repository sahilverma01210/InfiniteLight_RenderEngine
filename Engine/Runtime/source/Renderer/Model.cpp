#include "Model.h"

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
		return "Chili Model Exception";
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
					auto& transform = transforms[pSelectedNode->GetId()];
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

	Model::Model(D3D12RHI& gfx, const std::string fileName)
		:
		pWindow(std::make_unique<ModelWindow>())
	{
		Assimp::Importer imp;
		const auto pScene = imp.ReadFile(fileName.c_str(),
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
			meshPtrs.push_back(ParseMesh(gfx, *pScene->mMeshes[i], pScene->mMaterials));
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

	std::unique_ptr<Mesh> Model::ParseMesh(D3D12RHI& gfx, const aiMesh& mesh, const aiMaterial* const* pMaterials)
	{
		using namespace std::string_literals;

		UINT numSRVDescriptors = 0, srvDescriptorIndex = 0;

		std::unique_ptr<Bindable> psoBindablePtr;
		std::vector<std::shared_ptr<Bindable>> bindablePtrs;

		std::string diffPath, specPath, normPath;
		std::wstring vertexShaderName, pixelShaderName;

		std::string base("models\\gobber\\");

		bool hasSpecularMap = false;
		bool hasNormalMap = false;
		bool hasDiffuseMap = false;

		float shininess = 35.0f;
		const float scale = 1.0f;

		VertexRawBuffer vbuf;

		const void* pmc;

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
				diffPath = base + texFileName.C_Str();
				/*OutputDebugStringA(diffPath.c_str());
				OutputDebugStringA("\n");*/
				hasDiffuseMap = true;
				numSRVDescriptors++;
			}

			if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
			{
				specPath = base + texFileName.C_Str();
				/*OutputDebugStringA(specPath.c_str());
				OutputDebugStringA("\n");*/
				hasSpecularMap = true;
				numSRVDescriptors++;
			}
			else
			{
				material.Get(AI_MATKEY_SHININESS, shininess);
			}

			if (material.GetTexture(aiTextureType_NORMALS, 0, &texFileName) == aiReturn_SUCCESS)
			{
				normPath = base + texFileName.C_Str();
				/*OutputDebugStringA(normPath.c_str());
				OutputDebugStringA("\n");*/
				hasNormalMap = true;
				numSRVDescriptors++;
			}
		}

		if (hasDiffuseMap && hasNormalMap && hasSpecularMap)
		{
			vertexShaderName = L"PhongVSNormalMap.hlsl";
			pixelShaderName = L"PhongPSSpecNormalMap.hlsl";

			vbuf.SetLayout(VertexLayout{}
				.Append(VertexLayout::Position3D)
				.Append(VertexLayout::Normal)
				.Append(VertexLayout::Tangent)
				.Append(VertexLayout::Bitangent)
				.Append(VertexLayout::Texture2D));

			PSMaterialConstantFullmonte* temp_pmc = new PSMaterialConstantFullmonte();
			pmc = temp_pmc;

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

			PSMaterialConstantDiffnorm* temp_pmc = new PSMaterialConstantDiffnorm();
			temp_pmc->specularPower = shininess;
			pmc = temp_pmc;

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
		else if (hasDiffuseMap)
		{
			vertexShaderName = L"PhongVS.hlsl";
			pixelShaderName = L"PhongPS.hlsl";

			vbuf.SetLayout(VertexLayout{}
				.Append(VertexLayout::Position3D)
				.Append(VertexLayout::Normal)
				.Append(VertexLayout::Texture2D));

			PSMaterialConstantDiffuse* temp_pmc = new PSMaterialConstantDiffuse();
			temp_pmc->specularPower = shininess;
			pmc = temp_pmc;

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

			PSMaterialConstantNotex* temp_pmc = new PSMaterialConstantNotex();
			temp_pmc->specularPower = shininess;
			pmc = temp_pmc;

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

		// Add Pipeline State Obejct
		{
			ID3DBlob* vertexShader;
			ID3DBlob* pixelShader;

			// Compile Shaders.
			D3DCompileFromFile(gfx.GetAssetFullPath(vertexShaderName.c_str()).c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr);
			D3DCompileFromFile(gfx.GetAssetFullPath(pixelShaderName.c_str()).c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, nullptr);

			// Define the vertex input layout.
			std::vector<D3D12_INPUT_ELEMENT_DESC> vec = vbuf.GetLayout().GetD3DLayout();
			D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

			for (size_t i = 0; i < vec.size(); ++i) {
				inputElementDescs[i] = vec[i];
			}

			PipelineDescription pipelineDesc{ *vertexShader, *pixelShader, *inputElementDescs, vec.size(), 1, 2, numSRVDescriptors };

			psoBindablePtr = std::make_unique<PipelineState>(gfx, pipelineDesc);
		}

		auto pso = dynamic_cast<PipelineState*>(psoBindablePtr.get());

		// Add Other Bindables
		{
			bindablePtrs.push_back(std::make_shared<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
			bindablePtrs.push_back(std::make_shared<VertexBuffer>(gfx, vbuf.GetData(), UINT(vbuf.SizeBytes()), (UINT)vbuf.GetLayout().Size()));
			bindablePtrs.push_back(std::make_shared<IndexBuffer>(gfx, indices.size() * sizeof(indices[0]), indices));

			if (hasDiffuseMap)
			{
				bindablePtrs.push_back(std::make_shared<TextureBuffer>(gfx, 3, std::wstring(diffPath.begin(), diffPath.end()).c_str(), pso->GetSRVHeap(), srvDescriptorIndex));
				srvDescriptorIndex++;
			}

			if (hasSpecularMap)
			{
				bindablePtrs.push_back(std::make_shared<TextureBuffer>(gfx, 3, std::wstring(specPath.begin(), specPath.end()).c_str(), pso->GetSRVHeap(), srvDescriptorIndex));
				srvDescriptorIndex++;
			}

			if (hasNormalMap)
			{
				bindablePtrs.push_back(std::make_shared<TextureBuffer>(gfx, 3, std::wstring(normPath.begin(), normPath.end()).c_str(), pso->GetSRVHeap(), srvDescriptorIndex));
				srvDescriptorIndex++;
			}
		}

		bindablePtrs.push_back(std::make_shared<TransformBuffer>(gfx, 0));
		bindablePtrs.push_back(std::make_shared<ConstantBuffer>(gfx, 2, sizeof(pmc), &pmc));

		std::unique_ptr<Mesh> temp_mesh = std::make_unique<Mesh>(gfx, std::move(psoBindablePtr), std::move(bindablePtrs));
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