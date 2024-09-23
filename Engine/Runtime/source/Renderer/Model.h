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
		Model(D3D12RHI& gfx, const std::string fileName);
		void Draw(D3D12RHI& gfx) const noexcept;
		void ShowWindow(const char* windowName = nullptr) noexcept;
		static std::unique_ptr<Mesh> ParseMesh(D3D12RHI& gfx, const aiMesh& mesh, const aiMaterial* const* pMaterials);
		std::unique_ptr<Node> ParseNode(int& nextId, const aiNode& node) noexcept;
		~Model() noexcept;
	private:
		std::unique_ptr<Node> pRoot;
		std::vector<std::unique_ptr<Mesh>> meshPtrs;
		std::unique_ptr<class ModelWindow> pWindow;

		struct PSMaterialConstantFullmonte
		{
			BOOL  normalMapEnabled = TRUE;
			float padding[3];
		};

		struct PSMaterialConstantDiffnorm
		{
			float specularIntensity = 0.18f;
			float specularPower;
			BOOL  normalMapEnabled = TRUE;
			float padding[1];
		};

		struct PSMaterialConstantDiffuse
		{
			float specularIntensity = 0.18f;
			float specularPower;
			float padding[2];
		};

		struct PSMaterialConstantNotex
		{
			XMFLOAT4 materialColor = { 0.65f,0.65f,0.85f,1.0f };
			float specularIntensity = 0.18f;
			float specularPower;
			float padding[2];
		};
	};
}
