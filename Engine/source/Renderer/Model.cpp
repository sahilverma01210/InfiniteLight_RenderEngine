#include "Model.h"

namespace Renderer
{
	Model::Model(D3D12RHI& gfx, const std::string& pathString, float scale)
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
		std::vector<Material> materials;
		materials.reserve(pScene->mNumMaterials);
		for (size_t i = 0; i < pScene->mNumMaterials; i++)
		{
			materials.emplace_back(gfx, *pScene->mMaterials[i], pathString);
		}
		for (size_t i = 0; i < pScene->mNumMeshes; i++)
		{
			const auto& mesh = *pScene->mMeshes[i];
			m_meshPtrs.push_back(std::make_shared<Mesh>(gfx, materials[mesh.mMaterialIndex], mesh, scale));
		}

		int nextId = 0;
		m_pRoot = ParseNode(nextId, *pScene->mRootNode, scale);
	}

	void Model::Submit(size_t channels) const noexcept(!IS_DEBUG)
	{
		m_pRoot->Submit(channels, XMMatrixIdentity());
	}

	void Model::SetRootTransform(FXMMATRIX tf) noexcept(!IS_DEBUG)
	{
		m_pRoot->SetAppliedTransform(tf);
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
			pNode->AddChild(ParseNode(nextId, *node.mChildren[i], scale));
		}

		return pNode;
	}

	void Model::Accept(ModelProbe& probe)
	{
		m_pRoot->Accept(probe);
	}

	void Model::LinkTechniques(RenderGraph& rg)
	{
		for (auto& pMesh : m_meshPtrs)
		{
			pMesh->LinkTechniques(rg);
		}
	}
}