#include "Drawable.h"

namespace Renderer
{
	void Drawable::Bind(D3D12RHI& gfx, std::string targetPass) const noexcept(!IS_DEBUG)
	{
		m_topologyBindable->Bind(gfx);
		m_vertexBufferBindable->Bind(gfx);
		m_indexBufferBindable->Bind(gfx);
		m_rootSignBindables.at(targetPass)->Bind(gfx);
		m_psoBindables.at(targetPass)->Bind(gfx);

		SetTransform(gfx, targetPass);
	}

	void Drawable::Draw(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		gfx.DrawIndexed(m_numIndices);
	}

	void Drawable::AddTechnique(Technique tech_in) noexcept(!IS_DEBUG)
	{
		m_techniques.push_back(std::move(tech_in));
	}

	void Drawable::LinkTechniques(RenderGraph& rg)
	{
		for (auto& technique : m_techniques)
		{
			technique.Link(rg);
		}
	}

	void Drawable::Submit(size_t channel) const noexcept(!IS_DEBUG)
	{
		for (const auto& technique : m_techniques)
		{
			technique.Submit(*this, channel);
		}
	}

	void Drawable::Accept(TechniqueProbe& probe)
	{
		for (auto& technique : m_techniques)
		{
			technique.Accept(probe);
		}
	}
}