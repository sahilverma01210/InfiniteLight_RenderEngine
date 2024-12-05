#include "Node.h"

namespace Renderer
{
	Node::Node(int id, const std::string& name, std::vector<Mesh*> meshPtrs, const XMMATRIX& transform_in) noexcept
		:
		id(id),
		meshPtrs(std::move(meshPtrs)),
		name(name)
	{
		XMStoreFloat4x4(&transform, transform_in);
		XMStoreFloat4x4(&appliedTransform, XMMatrixIdentity());
	}

	void Node::Submit(size_t channels, FXMMATRIX accumulatedTransform) const noexcept
	{
		const auto built =
			XMLoadFloat4x4(&appliedTransform) *
			XMLoadFloat4x4(&transform) *
			accumulatedTransform;
		for (const auto pm : meshPtrs)
		{
			pm->Submit(channels, built);
		}
		for (const auto& pc : childPtrs)
		{
			pc->Submit(channels, built);
		}
	}

	void Node::AddChild(std::unique_ptr<Node> pChild) noexcept
	{
		assert(pChild);
		childPtrs.push_back(std::move(pChild));
	}

	void Node::SetAppliedTransform(FXMMATRIX transform) noexcept
	{
		XMStoreFloat4x4(&appliedTransform, transform);
	}

	const XMFLOAT4X4& Node::GetAppliedTransform() const noexcept
	{
		return appliedTransform;
	}

	int Node::GetId() const noexcept
	{
		return id;
	}

	void Node::Accept(ModelProbe& probe)
	{
		if (probe.PushNode(*this))
		{
			for (auto& cp : childPtrs)
			{
				cp->Accept(probe);
			}
			probe.PopNode(*this);
		}
	}

	void Node::Accept(TechniqueProbe& probe)
	{
		for (auto& mp : meshPtrs)
		{
			mp->Accept(probe);
		}
	}
}