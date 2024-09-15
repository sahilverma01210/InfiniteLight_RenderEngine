#pragma once
#include "../Common/ILException.h"
#include "Object.h"
#include "BindableCommon.h"
#include "Vertex.h"
#include <optional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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

	class Mesh : public Object<Mesh>
	{
	public:
		Mesh(D3D12RHI& gfx, std::vector<std::unique_ptr<Bindable>> bindPtrs);
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
		Node(int id, const std::string& name, std::vector<Mesh*> meshPtrs, const XMMATRIX& transform) noexcept;
		void Draw(D3D12RHI& gfx, FXMMATRIX accumulatedTransform) const noexcept;
		void SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept;
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
	};
}