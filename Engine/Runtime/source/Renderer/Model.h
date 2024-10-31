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
	class FrameCommander;
	class ModelWindow;

	class Model
	{
	public:
		Model(D3D12RHI& gfx, const std::string& pathString, float fscale = 1.0f);
		void Submit(FrameCommander& frame) const noexcept;
		void SetRootTransform(DirectX::FXMMATRIX tf) noexcept;
		void Accept(class ModelProbe& probe);
		~Model() noexcept;
	private:
		static std::unique_ptr<Mesh> ParseMesh(D3D12RHI& gfx, const aiMesh& mesh, const aiMaterial* const* pMaterials, const std::filesystem::path& path, float fscale);
		std::unique_ptr<Node> ParseNode(int& nextId, const aiNode& node, float scale) noexcept;
	private:
		std::unique_ptr<Node> pRoot;
		std::vector<std::unique_ptr<Mesh>> meshPtrs;
		//std::unique_ptr<class ModelWindow> pWindow;
	};
}