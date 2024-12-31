#pragma once
#include "../_External/assimp/assimp.h"

#include "Node.h"
#include "RenderMath.h"
#include "ModelException.h"

namespace Renderer
{
	class Model
	{
	public:
		Model(D3D12RHI& gfx, const std::string& pathString, XMFLOAT3 transform,  float fscale = 1.0f);
		void Submit(size_t channel) const noexcept(!IS_DEBUG);
		void Accept(class ModelProbe& probe);
		void LinkTechniques(RenderGraph&);
	private:
		std::unique_ptr<Node> ParseNode(int& nextId, const aiNode& node, float scale) noexcept(!IS_DEBUG);

	private:
		std::unique_ptr<Node> m_pRoot;
		std::vector<std::shared_ptr<Mesh>> m_meshPtrs;
	};
}