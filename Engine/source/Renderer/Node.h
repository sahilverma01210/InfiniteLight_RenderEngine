#pragma once
#include "../Common/ImGUI_Includes.h"

#include "Mesh.h"

namespace Renderer
{
	struct TransformParameters
	{
		float xRot = 0.0f;
		float yRot = 0.0f;
		float zRot = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	};

	class Node
	{
		friend class Model;

	public:
		Node(int id, const std::string& name, std::vector<std::shared_ptr<Mesh>> meshPtrs, const XMMATRIX& transform) noexcept(!IS_DEBUG);
		void SpawnWindow();
		void OnSelect() noexcept(!IS_DEBUG);
		void OnDeselect() noexcept(!IS_DEBUG);
		void Update(FXMMATRIX accumulatedTransform) const noexcept(!IS_DEBUG);
		void SetAppliedTransform(FXMMATRIX transform) noexcept(!IS_DEBUG);
		const XMFLOAT4X4& GetAppliedTransform() const noexcept(!IS_DEBUG);
		int GetId() const noexcept(!IS_DEBUG);
		bool HasChildren() const noexcept(!IS_DEBUG) { return m_childPtrs.size() > 0; }
		void Accept(Model& model);
		const std::string& GetName() const { return m_name; }
	private:
		void AddChild(std::unique_ptr<Node> pChild) noexcept(!IS_DEBUG);
		TransformParameters& ResolveTransform() noexcept(!IS_DEBUG);
		TransformParameters& LoadTransform(int id) noexcept(!IS_DEBUG);

	private:
		std::string m_name;
		int m_id;
		std::vector<std::unique_ptr<Node>> m_childPtrs;
		std::vector<std::shared_ptr<Mesh>> m_meshPtrs;
		XMFLOAT4X4 m_transform;
		XMFLOAT4X4 m_appliedTransform;
		std::unordered_map<int, TransformParameters> m_transformParams;
	};

}