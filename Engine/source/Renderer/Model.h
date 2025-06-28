#pragma once
#include "../_External/assimp/assimp.h"
#include "../Common/ImGUI_Includes.h"

#include "Node.h"
#include "RenderMath.h"
#include "ModelException.h"
#include "DynamicConstant.h"

namespace Renderer
{
	class Model
	{
	public:
		Model(D3D12RHI& gfx, std::string name, const std::string& pathString, XMFLOAT3 transform,  float fscale = 1.0f);
		bool SpawnWindow();
		bool PushNode(Node& node);
		void PopNode(Node& node);
		void Update() const noexcept(!IS_DEBUG);
		std::vector<std::shared_ptr<Mesh>> GetMeshes() const noexcept(!IS_DEBUG) { return m_meshPtrs; }
	private:
		std::unique_ptr<Node> ParseNode(int& nextId, const aiNode& node, float scale) noexcept(!IS_DEBUG);

	public:
		bool m_imGUIwndOpen = true;
	private:
		std::string m_name;
		Node* m_pSelectedNode = nullptr;
		std::unique_ptr<Node> m_pRoot;
		std::vector<std::shared_ptr<Mesh>> m_meshPtrs;
	};
}