#pragma once
#include "../_External/assimp/assimp.h"
#include "../Common/ImGUI_Includes.h"

#include "Node.h"
#include "RenderMath.h"
#include "ModelException.h"

namespace Renderer
{
	struct InstanceData
	{
		Matrix worldMatrix;
		Matrix inverseWorldMatrix;
		Uint32 instanceId;
		Uint32 meshIndex;
		Uint32 materialIndex;
	};

	struct MeshData
	{
		Uint32 bufferIdx;
		Uint32 positionsOffset;
		Uint32 uvsOffset;
		Uint32 normalsOffset;
		Uint32 tangentsOffset;
		Uint32 indicesOffset;
		Uint32 indicesCount;
		Uint32 verticesCount;
	};

	struct MaterialData
	{
		Vector3 pbrBaseColorFactor = Vector3(1.0f, 1.0f, 1.0f);
		float pbrMetallicFactor = 1.0f;
		float pbrRoughnessFactor = 1.0f;
		float gltfAlphaCutoff = 0.5f;
		ResourceHandle diffuseIdx = -1;
		ResourceHandle normalIdx = -1;
		ResourceHandle roughnessMetallicIdx = -1;
	};

	class Model
	{
	public:
		Model(D3D12RHI& gfx, std::string name, const std::string& pathString, Vector3 transform,  float fscale = 1.0f);
		bool SpawnWindow();
		bool PushNode(Node& node);
		void PopNode(Node& node);
		void Update() const noexcept(!IS_DEBUG);
		const std::vector<std::shared_ptr<Mesh>>& GetMeshPointers() const noexcept(!IS_DEBUG) { return m_meshPtrs; }
		const std::vector<InstanceData>& GetInstances() const noexcept(!IS_DEBUG) { return m_instances; }
		const std::vector<MeshData>& GetMeshes() const noexcept(!IS_DEBUG) { return m_meshes; }
		const std::vector<MaterialData>& GetMaterials() const noexcept(!IS_DEBUG) { return m_materials; }
		const std::shared_ptr<D3D12Buffer>& GetMeshCacheBuffer() const noexcept(!IS_DEBUG) { return m_modelCacheBuffer; }
	private:
		std::unique_ptr<Node> ParseNode(int& nextId, const aiNode& node, float scale) noexcept(!IS_DEBUG);

	public:
		bool m_imGUIwndOpen = true;
	private:
		std::string m_name;
		Node* m_pSelectedNode = nullptr;
		std::unique_ptr<Node> m_pRoot;
		std::vector<std::shared_ptr<Mesh>> m_meshPtrs;
		std::vector<InstanceData> m_instances;
		std::vector<MeshData> m_meshes;
		std::vector<MaterialData> m_materials;
		std::shared_ptr<D3D12Buffer> m_instanceDataBuffer;
		std::shared_ptr<D3D12Buffer> m_meshDataBuffer;
		std::shared_ptr<D3D12Buffer> m_materialDataBuffer;
		std::shared_ptr<D3D12Buffer> m_modelCacheBuffer;
	};
}