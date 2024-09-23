#pragma once
#include "../_External/framework.h"

#include "Drawable.h"
#include "CommonBindables.h"
#include "CommonShapes.h"
#include "UIManager.h"

namespace Renderer
{
	class Mesh : public Drawable
	{
	public:
		Mesh(D3D12RHI& gfx, std::unique_ptr<Bindable> psoBindPtr, std::vector<std::shared_ptr<Bindable>> bindPtrs);
		void Draw(D3D12RHI& gfx, FXMMATRIX accumulatedTransform) const noexcept;
		void SetNumIndices(UINT numIndices);
		const UINT GetNumIndices() const noexcept;
		XMMATRIX GetTransformXM() const noexcept override;
	private:
		UINT m_numIndices;
		mutable XMFLOAT4X4 transform;
	};

	class Node
	{
		friend class Model;
	public:
		Node(int id, const std::string& name, std::vector<Mesh*> meshPtrs, const XMMATRIX& transform) noexcept;
		void Draw(D3D12RHI& gfx, FXMMATRIX accumulatedTransform) const noexcept;
		void SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept;
		int GetId() const noexcept;
		void ShowTree(Node*& pSelectedNode) const noexcept;
	private:
		void AddChild(std::unique_ptr<Node> pChild) noexcept;
	private:
		std::string name;
		int id;
		std::vector<std::unique_ptr<Node>> childPtrs;
		std::vector<Mesh*> meshPtrs;
		DirectX::XMFLOAT4X4 transform;
		DirectX::XMFLOAT4X4 appliedTransform;
	};
}