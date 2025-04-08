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

	void ILMesh::ApplyMaterial(D3D12RHI& gfx, ILMaterial* material, bool enableLighting) noexcept(!IS_DEBUG)
	{
		m_materialIdx = material->GetMaterialHandle();

		for (auto& technique : material->GetTechniques())
		{
			AddTechnique(std::move(technique));
		}
	}

	void ILMesh::AddTechnique(Technique tech_in) noexcept(!IS_DEBUG)
	{
		m_techniques.push_back(std::move(tech_in));
	}

	void ILMesh::Submit(RenderGraph& renderGraph) const noexcept(!IS_DEBUG)
	{
		for (const auto& technique : m_techniques)
		{
			if (technique.active)
			{
				for (auto& passName : technique.passNames)
				{
					renderGraph.GetRenderQueue(passName).Accept(*this);
				}
			}
		}
	}

	void ILMesh::Accept(TechniqueProbe& probe)
	{
		for (auto& technique : m_techniques)
		{
			probe.SetTechnique(&technique);
		}
		m_postProcessEnabled = true;
	}
}