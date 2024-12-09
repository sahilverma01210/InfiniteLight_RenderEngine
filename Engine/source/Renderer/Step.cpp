#include "Step.h"
/*
* Every Step is linked to its corresponding Render Pass. Bind method of Bindables of this Step will be called while executing Render Pass linked to this Step.
* 
* PS: Step objects are linked before RenderGraph object. Hence we cannot include RenderGraph.h in Step.h
*/
#include "RenderGraph.h"

namespace Renderer
{
	void Step::Submit(const Drawable& drawable) const
	{
		pTargetPass->Accept(Job{ this,&drawable });
	}

	Step::Step(std::string targetPassName)
		:
		targetPassName{ std::move(targetPassName) }
	{}

	Step::Step(const Step& src) noexcept(!IS_DEBUG)
		:
		targetPassName(src.targetPassName)
	{
		bindables.reserve(src.bindables.size());
		for (auto& pb : src.bindables)
		{
			if (auto* pCloning = dynamic_cast<const TransformBuffer*>(pb.get()))
			{
				bindables.push_back(std::make_unique<TransformBuffer>(*pCloning));
			}
			else
			{
				bindables.push_back(pb);
			}
		}
	}

	void Step::AddBindable(std::shared_ptr<Bindable> bind_in) noexcept(!IS_DEBUG)
	{
		bindables.push_back(std::move(bind_in));
	}

	void Step::Bind(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		for (const auto& b : bindables)
		{
			b->Bind(gfx);
		}
	}

	void Step::Accept(TechniqueProbe& probe)
	{
		probe.SetStep(this);
		for (auto& pb : bindables)
		{
			pb->Accept(probe);
		}
	}

	std::vector<std::shared_ptr<Bindable>> Step::GetBindables() noexcept(!IS_DEBUG)
	{
		return bindables;
	}

	std::string Step::GetTargetPass() const
	{
		return targetPassName;
	}

	void Step::Link(RenderGraph& rg)
	{
		assert(pTargetPass == nullptr);
		pTargetPass = &rg.GetRenderQueue(targetPassName);
	}
}