#pragma once
#include "../_External/assimp/assimp.h"

#include "D3D12RHI.h"
#include "ModelException.h"
#include "Node.h"
#include "Mesh.h"
#include "Material.h"
#include "RenderMath.h"

namespace Renderer
{
	class Model
	{
	public:
		Model(D3D12RHI& gfx, const std::string& pathString, float fscale = 1.0f);
		~Model() noexcept(!IS_DEBUG);
		void Submit(size_t channels) const noexcept(!IS_DEBUG);
		void SetRootTransform(FXMMATRIX tf) noexcept(!IS_DEBUG);
		void Accept(class ModelProbe& probe);
		void LinkTechniques(RenderGraph&);
	private:
		static std::unique_ptr<Mesh> ParseMesh(D3D12RHI& gfx, const aiMesh& mesh, const aiMaterial* const* pMaterials, const std::filesystem::path& path, float fscale);
		std::unique_ptr<Node> ParseNode(int& nextId, const aiNode& node, float scale) noexcept(!IS_DEBUG);

	private:
		std::unique_ptr<Node> pRoot;
		std::vector<std::unique_ptr<Mesh>> meshPtrs;
	};
}