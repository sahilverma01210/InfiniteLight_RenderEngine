#include "Model.h"

namespace Renderer
{
	std::mutex mutex;

	Model::Model(D3D12RHI& gfx, std::string name, const std::string& pathString, XMFLOAT3 transform, float scale)
		:
		m_name(name)
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
			throw MDL_EXCEPTION(imp.GetErrorString());
		}

		// parse materials
		std::vector<std::shared_ptr<Material>> materials;
		materials.resize(pScene->mNumMaterials);
		{
			std::vector<std::thread> threads;
			for (size_t i = 0; i < pScene->mNumMaterials; i++)
			{
				threads.emplace_back([&materials, &gfx, pScene, i, pathString]() {
					std::lock_guard<std::mutex> guard(mutex);
					materials[i] = std::make_shared<Material>(gfx, *pScene->mMaterials[i], pathString, mutex);
					});

				// Assigning Name to each Threads.
				const char* matNamePtr = pScene->mMaterials[i]->GetName().C_Str();
				const std::wstring& matName = std::wstring(matNamePtr, matNamePtr + std::strlen(matNamePtr));
				SetThreadDescription(threads[i].native_handle(), matName.c_str());
			}
		
			for (auto& thread : threads)
			{
				thread.join();
			}
		}

		// parse materials
		//std::vector<std::shared_ptr<Material>> materials;
		//for (size_t i = 1; i < pScene->mNumMaterials; i++)
		//{
		//	//aiMaterial mat = *pScene->mMaterials[i];
		//	//std::string name = mat.GetName().C_Str();
		//	materials.push_back(std::make_shared<Material>(gfx, *pScene->mMaterials[i], pathString));
		//}

		//bool hasRootNode = false;

		//if (!(hasRootNode = pScene->mRootNode->mNumChildren))
		//{
		//	throw MDL_EXCEPTION("No root node found in the model.");
		//}

		aiNode** childNodes = new aiNode*[pScene->mNumMeshes];

		// parse mesh and apply materials
		for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
		{
			const auto& mesh = *pScene->mMeshes[i];
			childNodes[i] = new aiNode();
			childNodes[i]->mName = mesh.mName;
			childNodes[i]->mParent = pScene->mRootNode;
			childNodes[i]->mNumMeshes = 1;
			childNodes[i]->mMeshes = new unsigned int[1] {i};
			//std::string name = mesh.mName.C_Str();
			m_meshPtrs.push_back(std::make_shared<Mesh>(gfx, materials[mesh.mMaterialIndex], mesh, scale));
		}

		pScene->mRootNode->mNumChildren = pScene->mNumMeshes;
		pScene->mRootNode->mChildren = childNodes;

		int nextId = 0;
		m_pRoot = ParseNode(nextId, *pScene->mRootNode, scale);
		m_pRoot->SetAppliedTransform(XMMatrixTranslation(transform.x, transform.y, transform.z));
	}

	bool Model::SpawnWindow()
	{
		ImGui::Begin(m_name.c_str(), &m_imGUIwndOpen);
		ImGui::Columns(2, nullptr, true);
		m_pRoot->Accept(*this);
		ImGui::NextColumn();
		if (m_pSelectedNode != nullptr) m_pSelectedNode->SpawnWindow();
		ImGui::End();

		return m_imGUIwndOpen;
	}

	bool Model::PushNode(Node& node)
	{
		// if there is no selected node, set selectedId to an impossible value
		const int selectedId = (m_pSelectedNode == nullptr) ? -1 : m_pSelectedNode->GetId();
		// build up flags for current node
		const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow
			| ((node.GetId() == selectedId) ? ImGuiTreeNodeFlags_Selected : 0)
			| (node.HasChildren() ? 0 : ImGuiTreeNodeFlags_Leaf);
		// render this node
		const auto expanded = ImGui::TreeNodeEx(
			(void*)(intptr_t)node.GetId(),
			node_flags, node.GetName().c_str()
		);
		// processing for selecting node
		if (ImGui::IsItemClicked())
		{			
			if (m_pSelectedNode != nullptr) m_pSelectedNode->OnDeselect(); // Deselect prev-selected node
			node.OnSelect(); // Select newly-selected node
			m_pSelectedNode = &node;
		}
		// signal if children should also be recursed
		return expanded;
	}

	void Model::PopNode(Node& node)
	{
		ImGui::TreePop();
	}

	void Model::Update() const noexcept(!IS_DEBUG)
	{
		m_pRoot->Update(XMMatrixIdentity());
	}

	std::unique_ptr<Node> Model::ParseNode(int& nextId, const aiNode& node, float scale) noexcept(!IS_DEBUG)
	{
		const auto transform = ScaleTranslation(XMMatrixTranspose(XMLoadFloat4x4(
			reinterpret_cast<const XMFLOAT4X4*>(&node.mTransformation)
		)), scale);

		std::vector<std::shared_ptr<Mesh>> curMeshPtrs;
		curMeshPtrs.reserve(node.mNumMeshes);
		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			const auto meshIdx = node.mMeshes[i];
			curMeshPtrs.push_back(m_meshPtrs.at(meshIdx));
		}

		auto pNode = std::make_unique<Node>(nextId++, node.mName.C_Str(), std::move(curMeshPtrs), transform);
		for (size_t i = 0; i < node.mNumChildren; i++)
		{
			auto temp = node.mChildren[i];

			pNode->AddChild(ParseNode(nextId, *node.mChildren[i], scale));
		}

		return pNode;
	}
}