#include "Model.h"
#include "RenderGraph.h"

namespace Renderer
{
	std::mutex mutex;

	Model::Model(D3D12RHI& gfx, std::string name, const std::string& pathString, Vector3 transform, float scale)
		:
		m_name(name)
	{
		const auto rootPath = std::filesystem::path(pathString).parent_path().string() + "\\";

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
		m_materials.resize(pScene->mNumMaterials);
		m_meshes.resize(pScene->mNumMeshes);
		m_instances.resize(pScene->mNumMeshes);

		for (size_t i = 0; i < pScene->mNumMaterials; i++)
		{
			const auto& material = *pScene->mMaterials[i];

			// Load Texture Images
			{
				aiString diffFileName, normFileName, metalRoughFileName;

				TextureDesc desc{};
				desc.isSRGB = true;

				m_materials[i].diffuseIdx = gfx.LoadResource(std::make_shared<MeshTexture>(gfx, material.GetTexture(aiTextureType_DIFFUSE, 0, &diffFileName) == aiReturn_SUCCESS ? rootPath + diffFileName.C_Str() : "NULL_TEX", desc));
				m_materials[i].normalIdx = gfx.LoadResource(std::make_shared<MeshTexture>(gfx, material.GetTexture(aiTextureType_NORMALS, 0, &normFileName) == aiReturn_SUCCESS ? rootPath + normFileName.C_Str() : "NULL_TEX"));
				m_materials[i].roughnessMetallicIdx = gfx.LoadResource(std::make_shared<MeshTexture>(gfx, material.GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &metalRoughFileName) == aiReturn_SUCCESS ? rootPath + metalRoughFileName.C_Str() : "NULL_TEX"));
			}

			// Load Constant Buffers
			{
				aiColor4D baseColor(1, 1, 1, 1);

				material.Get(AI_MATKEY_GLTF_ALPHACUTOFF, m_materials[i].gltfAlphaCutoff);
				material.Get(AI_MATKEY_BASE_COLOR, baseColor);
				m_materials[i].pbrBaseColorFactor.x = baseColor.r;
				m_materials[i].pbrBaseColorFactor.y = baseColor.g;
				m_materials[i].pbrBaseColorFactor.z = baseColor.b;
				material.Get(AI_MATKEY_METALLIC_FACTOR, m_materials[i].pbrMetallicFactor);
				material.Get(AI_MATKEY_ROUGHNESS_FACTOR, m_materials[i].pbrRoughnessFactor);
				aiString alphaMode;
				material.Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
				if (alphaMode == aiString("OPAQUE")) m_materials[i].gltfAlphaMode = AlphaMode::Opaque;
				else if (alphaMode == aiString("MASK")) m_materials[i].gltfAlphaMode = AlphaMode::Mask;
				else if (alphaMode == aiString("BLEND")) m_materials[i].gltfAlphaMode = AlphaMode::Blend;
			}
		}

		aiNode** childNodes = new aiNode*[pScene->mNumMeshes];

		Uint64 totalBufferSize = 0;

		// parse mesh
		for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
		{
			const auto& mesh = *pScene->mMeshes[i];
			childNodes[i] = new aiNode();
			childNodes[i]->mName = mesh.mName;
			childNodes[i]->mParent = pScene->mRootNode;
			childNodes[i]->mNumMeshes = 1;
			childNodes[i]->mMeshes = new unsigned int[1] {i};
			m_meshPtrs.push_back(std::make_shared<Mesh>(gfx, mesh, scale));

			totalBufferSize += Align(mesh.mNumFaces * 3 * sizeof(Uint32), 16);
			totalBufferSize += Align(mesh.mNumVertices * sizeof(Vector3), 16);
			totalBufferSize += Align(mesh.mNumVertices * sizeof(Vector3), 16);
			totalBufferSize += Align(mesh.mNumVertices * sizeof(Vector3), 16);
			totalBufferSize += Align(mesh.mNumVertices * sizeof(Vector2), 16);

			m_instances[i].instanceId = i;
			m_instances[i].meshIndex = i;
			m_instances[i].materialIndex = mesh.mMaterialIndex;
		}

		pScene->mRootNode->mNumChildren = pScene->mNumMeshes;
		pScene->mRootNode->mChildren = childNodes;

		int nextId = 0;
		m_pRoot = ParseNode(nextId, *pScene->mRootNode, scale);
		m_pRoot->SetAppliedTransform(XMMatrixTranslation(transform.x, transform.y, transform.z));
		m_pRoot->Update(XMMatrixIdentity());

		// Load Acceleration Structure Data

		m_modelCacheBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, nullptr, totalBufferSize, 0, ResourceFlags::None, BufferType::Raw));
		ResourceHandle modelCacheHandle = gfx.LoadResource(m_modelCacheBuffer, D3D12Resource::ViewType::SRV);

		Uint32 currentOffset = 0;

		for (unsigned int i = 0; i < m_meshPtrs.size(); i++)
		{
			auto& drawData = m_meshPtrs[i]->GetDrawData();

			m_meshes[i].bufferIdx = modelCacheHandle;
			m_meshes[i].indicesCount = (Uint32)drawData.indices.size();
			m_meshes[i].verticesCount = (Uint32)drawData.vertices.size();

			std::vector<UINT32> indices;
			std::vector<Vector3> positions;
			std::vector<Vector3> normals;
			std::vector<Vector3> tangents;
			std::vector<Vector2> uvCoordinates;

			for (int j = 0; j < drawData.indices.size(); j++)
			{
				indices.emplace_back(drawData.indices[j]);
			}
			
			for (int i = 0; i < drawData.vertices.size(); i++)
			{
				auto position = drawData.vertices[i].position;
				auto normal = drawData.vertices[i].normal;
				auto tangent = drawData.vertices[i].tangent;
				auto uv = drawData.vertices[i].texCoord;
			
				positions.emplace_back(position.x, position.y, position.z);
				normals.emplace_back(normal.x, normal.y, normal.z);
				tangents.emplace_back(tangent.x, tangent.y, tangent.z);
				uvCoordinates.emplace_back(uv.x, uv.y);
			}

			m_meshes[i].indicesOffset = currentOffset;
			m_modelCacheBuffer->UpdateGPU(indices.data(), indices.size() * sizeof(indices[0]), currentOffset);
			currentOffset += (Uint32)Align(indices.size() * sizeof(indices[0]), 16);

			m_meshes[i].positionsOffset = currentOffset;
			m_modelCacheBuffer->UpdateGPU(positions.data(), positions.size() * sizeof(positions[0]), currentOffset);
			currentOffset += (Uint32)Align(positions.size() * sizeof(positions[0]), 16);

			m_meshes[i].normalsOffset = currentOffset;
			m_modelCacheBuffer->UpdateGPU(normals.data(), normals.size() * sizeof(normals[0]), currentOffset);
			currentOffset += (Uint32)Align(normals.size() * sizeof(normals[0]), 16);

			m_meshes[i].tangentsOffset = currentOffset;
			m_modelCacheBuffer->UpdateGPU(tangents.data(), tangents.size() * sizeof(tangents[0]), currentOffset);
			currentOffset += (Uint32)Align(tangents.size() * sizeof(tangents[0]), 16);

			m_meshes[i].uvsOffset = currentOffset;
			m_modelCacheBuffer->UpdateGPU(uvCoordinates.data(), uvCoordinates.size() * sizeof(uvCoordinates[0]), currentOffset);
			currentOffset += (Uint32)Align(uvCoordinates.size() * sizeof(uvCoordinates[0]), 16);

			m_instances[i].worldMatrix = m_meshPtrs[i]->GetTransforms().meshMat;
			m_instances[i].inverseWorldMatrix = m_meshPtrs[i]->GetTransforms().meshInvMat;
		}

		m_materialDataBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, m_materials.data(), sizeof(MaterialData) * m_materials.size(), sizeof(MaterialData)));
		RenderGraph::m_frameData.materialsHandle = gfx.LoadResource(m_materialDataBuffer, D3D12Resource::ViewType::SRV);

		m_meshDataBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, m_meshes.data(), sizeof(MeshData) * m_meshes.size(), sizeof(MeshData)));
		RenderGraph::m_frameData.meshesHandle = gfx.LoadResource(m_meshDataBuffer, D3D12Resource::ViewType::SRV);

		m_instanceDataBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, m_instances.data(), sizeof(InstanceData) * m_instances.size(), sizeof(InstanceData)));
		RenderGraph::m_frameData.instancesHandle = gfx.LoadResource(m_instanceDataBuffer, D3D12Resource::ViewType::SRV);
	}

	bool Model::SpawnWindow()
	{
		if (ImGui::Begin(m_name.c_str(), &m_imGUIwndOpen, ImGuiWindowFlags_NoBackground))
		{
			ImGui::Columns(2, nullptr, true);
			m_pRoot->Accept(*this);
			ImGui::NextColumn();
			if (m_pSelectedNode != nullptr) m_pSelectedNode->SpawnWindow();
		}
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

	// Update Disabled
	void Model::Update() const noexcept(!IS_DEBUG)
	{
		//m_pRoot->Update(XMMatrixIdentity());

		//for (unsigned int i = 0; i < m_meshPtrs.size(); i++)
		//{
		//	m_instances[i].worldMatrix = m_meshPtrs[i]->GetTransforms().meshMat;
		//	m_instances[i].inverseWorldMatrix = m_meshPtrs[i]->GetTransforms().meshInvMat;
		//}
		//m_instanceDataBuffer->Update(m_instances.data(), sizeof(InstanceData) * m_instances.size(), BufferType::Constant);
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