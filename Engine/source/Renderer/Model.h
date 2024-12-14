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
		Model(D3D12RHI& gfx, const std::string& pathString, float fscale = 1.0f);
		void Submit(size_t channels) const noexcept(!IS_DEBUG);
		void SetRootTransform(FXMMATRIX tf) noexcept(!IS_DEBUG);
		void Accept(class ModelProbe& probe);
		void LinkTechniques(RenderGraph&);
	private:
		std::unique_ptr<Node> ParseNode(int& nextId, const aiNode& node, float scale) noexcept(!IS_DEBUG);

	private:
		std::unique_ptr<Node> m_pRoot;
		std::vector<std::shared_ptr<Mesh>> m_meshPtrs;
	};
}