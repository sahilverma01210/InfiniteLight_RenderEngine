#include "Drawable.h"

namespace Renderer
{
	std::shared_ptr<Bindable> Drawable::lightBindable;
	std::shared_ptr<Bindable> Drawable::lightShadowBindable;

	void Drawable::Submit(size_t channels) const noexcept
	{
		for (const auto& tech : techniques)
		{
			tech.Submit(*this, channels);
		}
	}

	Drawable::Drawable(D3D12RHI& gfx, Material& mat, const aiMesh& mesh, float scale) noexcept
	{
		topologyBindable = Topology::Resolve(gfx);
		vertexBufferBindable = mat.MakeVertexBindable(gfx, mesh, scale);
		indexBufferBindable = mat.MakeIndexBindable(gfx, mesh);

		for (auto& pipelineDesc : mat.GetPipelineDesc())
		{
			rootSignBindables[pipelineDesc.first] = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc.second));
			psoBindables[pipelineDesc.first] = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc.second));
		}

		m_numIndices = indexBufferBindable->GetNumOfIndices();
		
		for (auto& t : mat.GetTechniques())
		{
			AddTechnique(std::move(t));
		}
	}

	void Drawable::AddTechnique(Technique tech_in) noexcept
	{
		tech_in.InitializeParentReferences(*this);
		techniques.push_back(std::move(tech_in));
	}

	void Drawable::Bind(D3D12RHI& gfx, std::string targetPass) const noexcept
	{
		topologyBindable->Bind(gfx);
		vertexBufferBindable->Bind(gfx);
		indexBufferBindable->Bind(gfx);
		rootSignBindables.at(targetPass)->Bind(gfx);
		psoBindables.at(targetPass)->Bind(gfx);
	}

	void Drawable::BindLighting(D3D12RHI& gfx) const noexcept
	{
		if (enableLighting)
		{
			lightBindable->Bind(gfx);
			lightShadowBindable->Bind(gfx);
		}
	}

	void Drawable::Accept(TechniqueProbe& probe)
	{
		for (auto& t : techniques)
		{
			t.Accept(probe);
		}
	}

	void Drawable::LinkTechniques(RenderGraph& rg)
	{
		for (auto& tech : techniques)
		{
			tech.Link(rg);
		}
	}

	UINT Drawable::GetIndexCount() const noexcept
	{
		return m_numIndices;
	}

	Drawable::~Drawable()
	{}
}