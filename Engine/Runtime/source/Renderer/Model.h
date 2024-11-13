#pragma once
#include "D3D12RHI.h"
#include <string>
#include <memory>
#include <filesystem>

struct aiMesh;
struct aiMaterial;
struct aiNode;

namespace Renderer
{
	class Node;
	class Mesh;
	class ModelWindow;
	class RenderGraph;

	class Model
	{
	public:
		Model(D3D12RHI& gfx, const std::string& pathString, float fscale = 1.0f);
		void Submit() const noexcept;
		void SetRootTransform(DirectX::FXMMATRIX tf) noexcept;
		void Accept(class ModelProbe& probe);
		void LinkTechniques(RenderGraph&);
		~Model() noexcept;
	private:
		static std::unique_ptr<Mesh> ParseMesh(D3D12RHI& gfx, const aiMesh& mesh, const aiMaterial* const* pMaterials, const std::filesystem::path& path, float fscale);
		std::unique_ptr<Node> ParseNode(int& nextId, const aiNode& node, float scale) noexcept;
	private:
		std::unique_ptr<Node> pRoot;
		std::vector<std::unique_ptr<Mesh>> meshPtrs;
	};
}