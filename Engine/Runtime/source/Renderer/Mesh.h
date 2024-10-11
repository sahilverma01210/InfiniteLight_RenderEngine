#pragma once
#include "../_External/framework.h"

#include "Drawable.h"
#include "CommonBindables.h"
#include "CommonShapes.h"
#include "UIManager.h"
#include "RenderMath.h"
#include "DynamicConstant.h"

namespace Renderer
{
	class Mesh : public Drawable
	{
	public:
		Mesh(D3D12RHI& gfx, std::unique_ptr<Bindable> rootSignBindablePtr, std::unique_ptr<Bindable> psoBindPtr, std::unique_ptr<Bindable> srvBindPtr, std::vector<std::shared_ptr<Bindable>> bindPtrs);
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
		struct PSMaterialConstantFullmonte
		{
			BOOL  normalMapEnabled = TRUE;
			BOOL  specularMapEnabled = TRUE;
			BOOL  hasGlossMap = FALSE;
			float specularPower = 3.1f;
			DirectX::XMFLOAT3 specularColor = { 0.75f,0.75f,0.75f };
			float specularMapWeight = 0.671f;
		};
		struct PSMaterialConstantNotex
		{
			DirectX::XMFLOAT4 materialColor = { 0.447970f,0.327254f,0.176283f,1.0f };
			DirectX::XMFLOAT4 specularColor = { 0.65f,0.65f,0.65f,1.0f };
			float specularPower = 120.0f;
			float padding[3];
		};
		Node(int id, const std::string& name, std::vector<Mesh*> meshPtrs, const XMMATRIX& transform) noexcept;
		void Draw(D3D12RHI& gfx, FXMMATRIX accumulatedTransform) const noexcept;
		void SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept;
		const DirectX::XMFLOAT4X4& GetAppliedTransform() const noexcept;
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