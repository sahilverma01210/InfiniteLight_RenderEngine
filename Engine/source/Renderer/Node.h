#pragma once
#include "../Common/ImGUI_Includes.h"

#include "Mesh.h"
#include "ModelProbe.h"

namespace Renderer
{
	class Node
	{
		friend class Model;

	public:
		Node(int id, const std::string& name, std::vector<std::shared_ptr<Mesh>> meshPtrs, const XMMATRIX& transform) noexcept(!IS_DEBUG);
		void Submit(FXMMATRIX accumulatedTransform, RenderGraph& renderGraph) const noexcept(!IS_DEBUG);
		void SetAppliedTransform(FXMMATRIX transform) noexcept(!IS_DEBUG);
		const XMFLOAT4X4& GetAppliedTransform() const noexcept(!IS_DEBUG);
		int GetId() const noexcept(!IS_DEBUG);
		bool HasChildren() const noexcept(!IS_DEBUG)
		{
			return m_childPtrs.size() > 0;
		}
		void Accept(ModelProbe& probe);
		void Accept(TechniqueProbe& probe);
		const std::string& GetName() const
		{
			return m_name;
		}
		void ToggleEffect(std::string name, bool enabled) noexcept(!IS_DEBUG);
	private:
		void AddChild(std::unique_ptr<Node> pChild) noexcept(!IS_DEBUG);

	private:
		std::string m_name;
		int m_id;
		std::vector<std::unique_ptr<Node>> m_childPtrs;
		std::vector<std::shared_ptr<Mesh>> m_meshPtrs;
		XMFLOAT4X4 m_transform;
		XMFLOAT4X4 m_appliedTransform;
	};

}