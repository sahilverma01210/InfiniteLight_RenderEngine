#pragma once
#include "D3D12RHI.h"
#include "CommonBindables.h"
#include "Technique.h"
#include "Vertex.h"
#include "DynamicConstant.h"
#include "TransformBuffer.h"
#include "Channels.h"

struct aiMaterial;
struct aiMesh;

namespace Renderer
{
	class Material
	{
	public:
		Material(D3D12RHI& gfx, const aiMaterial& material, const std::filesystem::path& path) noexcept;
		VertexRawBuffer ExtractVertices(const aiMesh& mesh) const noexcept;
		std::vector<unsigned short> ExtractIndices(const aiMesh& mesh) const noexcept;
		std::shared_ptr<VertexBuffer> MakeVertexBindable(D3D12RHI& gfx, const aiMesh& mesh, float scale = 1.0f) const noexcept;
		std::shared_ptr<IndexBuffer> MakeIndexBindable(D3D12RHI& gfx, const aiMesh& mesh) const noexcept;
		std::unordered_map<std::string, PipelineDescription> GetPipelineDesc() noexcept;
		std::vector<Technique> GetTechniques() noexcept;
	private:
		std::string MakeMeshTag(const aiMesh& mesh) const noexcept;

	private:
		VertexLayout vtxLayout;
		std::unordered_map<std::string, PipelineDescription> pipelineDesc;
		std::vector<Technique> techniques;
		std::string modelPath;
		std::string name;
	};
}