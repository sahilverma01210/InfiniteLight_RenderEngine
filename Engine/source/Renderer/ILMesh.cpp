#include "ILMesh.h"

namespace Renderer
{
	void ILMesh::ApplyMesh(D3D12RHI& gfx, VertexRawBuffer vertices, std::vector<USHORT> indices, D3D12_PRIMITIVE_TOPOLOGY topology)
	{
		m_topologyBindable = std::move(std::make_shared<Topology>(gfx, topology));
		m_vertexBufferBindable = std::move(std::make_shared<VertexBuffer>(gfx, vertices.GetData(), UINT(vertices.SizeBytes()), (UINT)vertices.GetLayout().Size()));
		m_indexBufferBindable = std::move(std::make_shared<IndexBuffer>(gfx, indices));

		m_numIndices = m_indexBufferBindable->GetNumOfIndices();
	}

	void ILMesh::ApplyMaterial(D3D12RHI& gfx, ILMaterial& material, bool enableLighting) noexcept(!IS_DEBUG)
	{
		m_enableLighting = enableLighting;

		for (auto& pipelineDesc : material.GetPipelineDesc())
		{
			m_rootSignBindables[pipelineDesc.first] = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc.second));
			m_psoBindables[pipelineDesc.first] = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc.second));
		}

		for (auto& technique : material.GetTechniques())
		{
			AddTechnique(std::move(technique));
		}
	}
	void ILMesh::Submit(size_t channel) const noexcept(!IS_DEBUG)
	{
		Drawable::Submit(channel);
	}
}