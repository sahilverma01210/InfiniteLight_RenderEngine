#include "Mesh.h"
#include "imgui/imgui.h"
#include <unordered_map>
#include <sstream>

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

	// Mesh Definitions.

	Mesh::Mesh(D3D12RHI& gfx, std::vector<std::unique_ptr<Bindable>> bindPtrs)
	{
		if (!IsStaticInitialized())
		{
			//AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		}

		for (auto& pb : bindPtrs)
		{
			AddBindable(std::move(pb));
		}

		//AddBind(std::make_unique<TransformCbuf>(gfx, *this));
	}

	void Mesh::Draw(D3D12RHI& gfx, FXMMATRIX accumulatedTransform) const noexcept
	{
		XMStoreFloat4x4(&transform, accumulatedTransform);
		Drawable::Draw(gfx, GetTransformXM());
	}

	void Mesh::SetNumIndices(UINT numIndices)
	{
		m_numIndices = numIndices;
	}

	const UINT Mesh::GetNumIndices() const noexcept
	{
		return m_numIndices;
	}

	XMMATRIX Mesh::GetTransformXM() const noexcept
	{
		return XMLoadFloat4x4(&transform);
	}

	// Node Definitions.

	Node::Node(int id, const std::string& name, std::vector<Mesh*> meshPtrs, const XMMATRIX& transform_in) noexcept
		:
		id(id),
		meshPtrs(std::move(meshPtrs)),
		name(name)
	{
		XMStoreFloat4x4(&transform, transform_in);
		XMStoreFloat4x4(&appliedTransform, XMMatrixIdentity());
	}

	void Node::Draw(D3D12RHI& gfx, FXMMATRIX accumulatedTransform) const noexcept
	{
		const auto built =
			XMLoadFloat4x4(&appliedTransform) *
			XMLoadFloat4x4(&transform) *
			accumulatedTransform;
		for (const auto pm : meshPtrs)
		{
			pm->Draw(gfx, built);
		}
		for (const auto& pc : childPtrs)
		{
			pc->Draw(gfx, built);
		}
	}

	void Node::AddChild(std::unique_ptr<Node> pChild) noexcept
	{
		assert(pChild);
		childPtrs.push_back(std::move(pChild));
	}

	void Node::ShowTree(Node*& pSelectedNode) const noexcept
	{
		// if there is no selected node, set selectedId to an impossible value
		const int selectedId = (pSelectedNode == nullptr) ? -1 : pSelectedNode->GetId();
		// build up flags for current node
		const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow
			| ((GetId() == selectedId) ? ImGuiTreeNodeFlags_Selected : 0)
			| ((childPtrs.size() == 0) ? ImGuiTreeNodeFlags_Leaf : 0);

		// render this node
		const auto expanded = ImGui::TreeNodeEx(
			(void*)(intptr_t)GetId(), node_flags, name.c_str()
		);
		// processing for selecting node
		if (ImGui::IsItemClicked())
		{
			pSelectedNode = const_cast<Node*>(this);
		}
		// recursive rendering of open node's children
		if (expanded)
		{
			for (const auto& pChild : childPtrs)
			{
				pChild->ShowTree(pSelectedNode);
			}
			ImGui::TreePop();
		}
	}

	void Node::SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept
	{
		XMStoreFloat4x4(&appliedTransform, transform);
	}

	int Node::GetId() const noexcept
	{
		return id;
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
			aiProcess_GenNormals
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
		using VertexSpace::VertexLayout;
		VertexSpace::VertexBuffer vbuf(std::move(
			VertexLayout{}
			.Append(VertexLayout::Position3D)
			.Append(VertexLayout::Normal)
			.Append(VertexLayout::Texture2D)
		));

		auto& material = *pMaterials[mesh.mMaterialIndex];

		for (int i = 0; i < material.mNumProperties; i++)
		{
			auto& prop = *material.mProperties[i];
			int qqq = 90;
		}

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			vbuf.EmplaceBack(
				*reinterpret_cast<XMFLOAT3*>(&mesh.mVertices[i]),
				*reinterpret_cast<XMFLOAT3*>(&mesh.mNormals[i]),
				*reinterpret_cast<XMFLOAT2*>(&mesh.mTextureCoords[0][i])
			);
		}

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

		std::string base("models\\nano_textured\\");

		std::vector<std::unique_ptr<Bindable>> bindablePtrs;

		bool hasSpecularMap = false;
		std::string diffPath, specPath;
		float shininess = 35.0f;

		if (mesh.mMaterialIndex >= 0)
		{
			using namespace std::string_literals;
			auto& material = *pMaterials[mesh.mMaterialIndex];

			aiString texFileName;

			material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName);
			diffPath = base + texFileName.C_Str();

			if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
			{
				specPath = base + texFileName.C_Str();
				hasSpecularMap = true;
			}
			else
			{
				material.Get(AI_MATKEY_SHININESS, shininess);
			}
		}

		// Add Pipeline State Obejct
		{
			ID3DBlob* vertexShader;
			ID3DBlob* pixelShader;

			// Compile Shaders.
			D3DCompileFromFile(gfx.GetAssetFullPath(L"PhongVS.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr);
			D3DCompileFromFile(gfx.GetAssetFullPath(hasSpecularMap ? L"PhongPSSpecMap.hlsl" : L"PhongPS.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, nullptr);

			// Define the vertex input layout.
			std::vector<D3D12_INPUT_ELEMENT_DESC> vec = vbuf.GetLayout().GetD3DLayout();
			D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

			for (size_t i = 0; i < vec.size(); ++i) {
				inputElementDescs[i] = vec[i];
			}

			PipelineDescription pipelineDesc{ *vertexShader, *pixelShader, *inputElementDescs, vec.size(), 1, 2, 2 };

			bindablePtrs.push_back(std::make_unique<PipelineState>(gfx, pipelineDesc));
		}

		auto pso = dynamic_cast<PipelineState*>(bindablePtrs[0].get());
		
		// Add Other Bindables
		{
			bindablePtrs.push_back(std::make_unique<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
			bindablePtrs.push_back(std::make_unique<VertexBuffer<VertexSpace::VertexStruct>>(gfx, vbuf));
			bindablePtrs.push_back(std::make_unique<IndexBuffer>(gfx, indices.size() * sizeof(indices[0]), indices));
			bindablePtrs.push_back(std::make_unique<TextureBuffer>(gfx, 3, std::wstring(diffPath.begin(), diffPath.end()).c_str(), pso->GetSRVHeap(), 0));
			if (hasSpecularMap) bindablePtrs.push_back(std::make_unique<TextureBuffer>(gfx, 3, std::wstring(specPath.begin(), specPath.end()).c_str(), pso->GetSRVHeap(), 1));
		}

		bindablePtrs.push_back(std::make_unique<TransformBuffer>(gfx, 0));
		struct PSMaterialConstant
		{
			float specularIntensity = 0.8f;
			float specularPower;
			float padding[2];
		} pmc;
		pmc.specularPower = shininess;
		bindablePtrs.push_back(std::make_unique<ConstantBuffer>(gfx, 2, sizeof(pmc), &pmc));

		std::unique_ptr<Mesh> temp_mesh = std::make_unique<Mesh>(gfx, std::move(bindablePtrs));
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