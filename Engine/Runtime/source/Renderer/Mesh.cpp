#include "Mesh.h"

namespace Renderer
{
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

	const DirectX::XMFLOAT4X4& Node::GetAppliedTransform() const noexcept
	{
		return appliedTransform;
	}

	int Node::GetId() const noexcept
	{
		return id;
	}

	// Mesh Definitions.

	Mesh::Mesh(D3D12RHI& gfx, std::unique_ptr<Bindable> rootSignBindablePtr, std::unique_ptr<Bindable> psoBindPtr, std::unique_ptr<Bindable> srvBindPtr, std::vector<std::shared_ptr<Bindable>> bindPtrs)
	{
		AddRootSignatureObject(std::move(rootSignBindablePtr));
		AddPipelineStateObject(std::move(psoBindPtr));
		if (srvBindPtr != nullptr) AddShaderResourceViewObject(std::move(srvBindPtr));

		for (auto& pb : bindPtrs)
		{
			AddBindable(std::move(pb));
		}
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
}