#include "Step.h"
/*
* Every Step is linked to its corresponding Render Pass. Bind method of Bindables of this Step will be called while executing Render Pass linked to this Step.
* 
* PS: Step objects are linked before RenderGraph object. Hence we cannot include RenderGraph.h in Step.h
*/
#include "RenderGraph.h"

namespace Renderer
{
	Step::Step(std::string targetPassName)
		:
		m_targetPassName{ std::move(targetPassName) }
	{}

	Step::Step(const Step& src) noexcept(!IS_DEBUG)
		:
		m_targetPassName(src.m_targetPassName)
	{
		m_bindables.reserve(src.m_bindables.size());
		for (auto& pb : src.m_bindables)
		{
			m_bindables.push_back(pb);
		}
	}

	void Step::AddBindable(std::shared_ptr<Bindable> bind_in) noexcept(!IS_DEBUG)
	{
		m_bindables.push_back(bind_in);
	}

	void Step::Bind(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		for (const auto& bindable : m_bindables)
		{
			bindable->Bind(gfx);
		}
	}

	std::string Step::GetTargetPass() const
	{
		return m_targetPassName;
	}

	void Step::Link(RenderGraph& rg)
	{
		assert(m_pTargetPass == nullptr);
		m_pTargetPass = &rg.GetRenderQueue(m_targetPassName);
	}

	void Step::Submit(const Drawable& drawable) const
	{
		m_pTargetPass->Accept(Job{ this,&drawable });
	}

	void Step::Accept(TechniqueProbe& probe)
	{
		probe.SetStep(this);
		for (auto& bindable : m_bindables)
		{
			bindable->Accept(probe);
		}
	}
}