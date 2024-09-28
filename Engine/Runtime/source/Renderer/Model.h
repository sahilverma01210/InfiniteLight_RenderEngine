#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../_External/framework.h"
#include "../Common/ILException.h"

#include "Mesh.h"

using namespace Common;

namespace Renderer
{
	class ModelException : public ILException
	{
	public:
		ModelException(int line, const char* file, std::string note) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		const std::string& GetNote() const noexcept;
	private:
		std::string note;
	};

	class Model
	{
	public:
		Model(D3D12RHI& gfx, const std::string& pathString, float fscale = 1.0f);
		void Draw(D3D12RHI& gfx) const noexcept;
		void ShowWindow(const char* windowName = nullptr) noexcept;
		void SetRootTransform(DirectX::FXMMATRIX tf) noexcept;
		~Model() noexcept;
	private:
		static std::unique_ptr<Mesh> ParseMesh(D3D12RHI& gfx, const aiMesh& mesh, const aiMaterial* const* pMaterials, const std::filesystem::path& path, float fscale);
		std::unique_ptr<Node> ParseNode(int& nextId, const aiNode& node) noexcept;
	private:
		std::unique_ptr<Node> pRoot;
		std::vector<std::unique_ptr<Mesh>> meshPtrs;
		std::unique_ptr<class ModelWindow> pWindow;
	};
}
