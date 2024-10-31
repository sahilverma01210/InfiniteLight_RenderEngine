#include "Drawable.h"
#include "Material.h"

namespace Renderer
{
	std::shared_ptr<Bindable> Drawable::lightBindable;

	void Drawable::Submit(FrameCommander& frame) const noexcept
	{
		for (const auto& tech : techniques)
		{
			tech.Submit(frame, *this);
		}
	}

	Drawable::Drawable(D3D12RHI& gfx, Material& mat, const aiMesh& mesh, float scale) noexcept
	{
		topologyBindable = Topology::Resolve(gfx);
		vertexBufferBindable = mat.MakeVertexBindable(gfx, mesh, scale);
		indexBufferBindable = mat.MakeIndexBindable(gfx, mesh);

		for (auto& pipelineDesc : mat.GetPipelineDesc())
		{
			rootSignBindables.push_back(std::move(std::make_unique<RootSignature>(gfx, pipelineDesc)));
			psoBindables.push_back(std::move(std::make_unique<PipelineState>(gfx, pipelineDesc)));
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

	void Drawable::Bind(D3D12RHI& gfx, size_t targetPass) const noexcept
	{
		topologyBindable->Bind(gfx);
		vertexBufferBindable->Bind(gfx);
		indexBufferBindable->Bind(gfx);
		rootSignBindables[targetPass]->Bind(gfx);
		psoBindables[targetPass]->Bind(gfx);
	}

	void Drawable::BindLighting(D3D12RHI& gfx) const noexcept
	{
		lightBindable->Bind(gfx);
	}

	void Drawable::Accept(TechniqueProbe& probe)
	{
		for (auto& t : techniques)
		{
			t.Accept(probe);
		}
	}

	UINT Drawable::GetIndexCount() const noexcept
	{
		return m_numIndices;
	}

	Drawable::~Drawable()
	{}
}