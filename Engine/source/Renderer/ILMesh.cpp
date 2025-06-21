#include "ILMesh.h"

namespace Renderer
{
	void ILMesh::ApplyMesh(D3D12RHI& gfx, VertexRawBuffer vertices, std::vector<USHORT> indices)
	{
		m_numIndices = indices.size();
		m_vertexSizeInBytes = UINT(vertices.SizeBytes());
		m_indexSizeInBytes = m_numIndices * sizeof(indices[0]);
		m_vertexStrideInBytes = (UINT)vertices.GetLayout().Size();

		m_vertexBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, vertices.GetData(), m_vertexSizeInBytes));
		m_indexBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, indices.data(), m_indexSizeInBytes));
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