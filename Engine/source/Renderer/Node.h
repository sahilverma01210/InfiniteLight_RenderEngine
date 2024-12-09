#pragma once
#include "Mesh.h"
#include "ModelProbe.h"
#include "UIManager.h"

namespace Renderer
{
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
			XMFLOAT3 specularColor = { 0.75f,0.75f,0.75f };
			float specularMapWeight = 0.671f;
		};
		struct PSMaterialConstantNotex
		{
			XMFLOAT4 materialColor = { 0.447970f,0.327254f,0.176283f,1.0f };
			XMFLOAT4 specularColor = { 0.65f,0.65f,0.65f,1.0f };
			float specularPower = 120.0f;
			float padding[3];
		};

	public:
		Node(int id, const std::string& name, std::vector<Mesh*> meshPtrs, const XMMATRIX& transform) noexcept(!IS_DEBUG);
		void Submit(size_t channels, FXMMATRIX accumulatedTransform) const noexcept(!IS_DEBUG);
		void SetAppliedTransform(FXMMATRIX transform) noexcept(!IS_DEBUG);
		const XMFLOAT4X4& GetAppliedTransform() const noexcept(!IS_DEBUG);
		int GetId() const noexcept(!IS_DEBUG);
		bool HasChildren() const noexcept(!IS_DEBUG)
		{
			return childPtrs.size() > 0;
		}
		void Accept(ModelProbe& probe);
		void Accept(TechniqueProbe& probe);
		const std::string& GetName() const
		{
			return name;
		}
	private:
		void AddChild(std::unique_ptr<Node> pChild) noexcept(!IS_DEBUG);

	private:
		std::string name;
		int id;
		std::vector<std::unique_ptr<Node>> childPtrs;
		std::vector<Mesh*> meshPtrs;
		XMFLOAT4X4 transform;
		XMFLOAT4X4 appliedTransform;
	};

}