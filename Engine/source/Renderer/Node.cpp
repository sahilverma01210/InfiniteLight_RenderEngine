#include "Node.h"

namespace Renderer
{
	Node::Node(int id, const std::string& name, std::vector<std::shared_ptr<Mesh>> meshPtrs, const XMMATRIX& transform_in) noexcept(!IS_DEBUG)
		:
		m_id(id),
		m_meshPtrs(std::move(meshPtrs)),
		m_name(name)
	{
		XMStoreFloat4x4(&m_transform, transform_in);
		XMStoreFloat4x4(&m_appliedTransform, XMMatrixIdentity());
	}

	void Node::Submit(FXMMATRIX accumulatedTransform, RenderGraph& renderGraph) const noexcept(!IS_DEBUG)
	{
		const auto built =
			XMLoadFloat4x4(&m_appliedTransform) *
			XMLoadFloat4x4(&m_transform) *
			accumulatedTransform;
		for (const auto& pm : m_meshPtrs)
		{
			pm->Submit(built, renderGraph);
		}
		for (const auto& pc : m_childPtrs)
		{
			pc->Submit(built, renderGraph);
		}
	}

	void Node::ToggleEffect(std::string name, bool enabled) noexcept(!IS_DEBUG)
	{
		for (auto& mp : m_meshPtrs)
		{
			mp->ToggleEffect(name, enabled);
		}
	}

	void Node::AddChild(std::unique_ptr<Node> pChild) noexcept(!IS_DEBUG)
	{
		assert(pChild);
		m_childPtrs.push_back(std::move(pChild));
	}

	void Node::SetAppliedTransform(FXMMATRIX transform) noexcept(!IS_DEBUG)
	{
		XMStoreFloat4x4(&m_appliedTransform, transform);
	}

	const XMFLOAT4X4& Node::GetAppliedTransform() const noexcept(!IS_DEBUG)
	{
		return m_appliedTransform;
	}

	int Node::GetId() const noexcept(!IS_DEBUG)
	{
		return m_id;
	}

	void Node::Accept(ModelProbe& probe)
	{
		if (probe.PushNode(*this))
		{
			for (auto& cp : m_childPtrs)
			{
				cp->Accept(probe);
			}
			probe.PopNode(*this);
		}
	}

	void Node::Accept(TechniqueProbe& probe)
	{
		for (auto& mp : m_meshPtrs)
		{
			mp->Accept(probe);
		}
	}
}