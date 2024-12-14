#pragma once
#include "CommonBindables.h"
#include "Technique.h"
#include "Vertex.h"
#include "DynamicConstant.h"

struct aiMaterial;
struct aiMesh;

namespace Renderer
{
	class Material
	{
	public:
		Material(D3D12RHI& gfx, const aiMaterial& material, const std::filesystem::path& path) noexcept(!IS_DEBUG);
		VertexRawBuffer ExtractVertices(const aiMesh& mesh) const noexcept(!IS_DEBUG);
		std::vector<unsigned short> ExtractIndices(const aiMesh& mesh) const noexcept(!IS_DEBUG);
		std::shared_ptr<VertexBuffer> MakeVertexBindable(D3D12RHI& gfx, const aiMesh& mesh, float scale = 1.0f) const noexcept(!IS_DEBUG);
		std::shared_ptr<IndexBuffer> MakeIndexBindable(D3D12RHI& gfx, const aiMesh& mesh) const noexcept(!IS_DEBUG);
		std::unordered_map<std::string, PipelineDescription> GetPipelineDesc() noexcept(!IS_DEBUG);
		std::vector<Technique> GetTechniques() noexcept(!IS_DEBUG);
	private:
		std::string MakeMeshTag(const aiMesh& mesh) const noexcept(!IS_DEBUG);

	private:
		VertexLayout m_vtxLayout;
		std::unordered_map<std::string, PipelineDescription> m_pipelineDesc;
		std::vector<Technique> m_techniques;
		std::string m_modelPath;
		std::string m_name;
	};
}