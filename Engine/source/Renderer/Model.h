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
		~Model() noexcept;
		void Submit(size_t channels) const noexcept;
		void SetRootTransform(FXMMATRIX tf) noexcept;
		void Accept(class ModelProbe& probe);
		void LinkTechniques(RenderGraph&);
	private:
		static std::unique_ptr<Mesh> ParseMesh(D3D12RHI& gfx, const aiMesh& mesh, const aiMaterial* const* pMaterials, const std::filesystem::path& path, float fscale);
		std::unique_ptr<Node> ParseNode(int& nextId, const aiNode& node, float scale) noexcept;

	private:
		std::unique_ptr<Node> pRoot;
		std::vector<std::unique_ptr<Mesh>> meshPtrs;
	};
}