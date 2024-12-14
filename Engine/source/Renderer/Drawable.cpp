#include "Drawable.h"

namespace Renderer
{
	std::shared_ptr<Bindable> Drawable::m_lightBindable;
	std::shared_ptr<Bindable> Drawable::m_lightShadowBindable;

	void Drawable::Submit(size_t channels) const noexcept(!IS_DEBUG)
	{
		for (const auto& tech : m_techniques)
		{
			tech.Submit(*this, channels);
		}
	}

	Drawable::Drawable(D3D12RHI& gfx, Material& mat, const aiMesh& mesh, float scale) noexcept(!IS_DEBUG)
	{
		m_topologyBindable = Topology::Resolve(gfx);
		m_vertexBufferBindable = mat.MakeVertexBindable(gfx, mesh, scale);
		m_indexBufferBindable = mat.MakeIndexBindable(gfx, mesh);

		for (auto& pipelineDesc : mat.GetPipelineDesc())
		{
			m_rootSignBindables[pipelineDesc.first] = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc.second));
			m_psoBindables[pipelineDesc.first] = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc.second));
		}

		m_numIndices = m_indexBufferBindable->GetNumOfIndices();
		
		for (auto& t : mat.GetTechniques())
		{
			AddTechnique(std::move(t));
		}
	}

	void Drawable::AddTechnique(Technique tech_in) noexcept(!IS_DEBUG)
	{
		m_techniques.push_back(std::move(tech_in));
	}

	void Drawable::Bind(D3D12RHI& gfx, std::string targetPass) const noexcept(!IS_DEBUG)
	{
		m_topologyBindable->Bind(gfx);
		m_vertexBufferBindable->Bind(gfx);
		m_indexBufferBindable->Bind(gfx);
		m_rootSignBindables.at(targetPass)->Bind(gfx);
		m_psoBindables.at(targetPass)->Bind(gfx);
	}

	void Drawable::BindLighting(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		if (m_enableLighting)
		{
			m_lightBindable->Bind(gfx);
			m_lightShadowBindable->Bind(gfx);
		}
	}

	void Drawable::Accept(TechniqueProbe& probe)
	{
		for (auto& t : m_techniques)
		{
			t.Accept(probe);
		}
	}

	void Drawable::LinkTechniques(RenderGraph& rg)
	{
		for (auto& tech : m_techniques)
		{
			tech.Link(rg);
		}
	}

	UINT Drawable::GetIndexCount() const noexcept(!IS_DEBUG)
	{
		return m_numIndices;
	}

	Drawable::~Drawable()
	{}
}