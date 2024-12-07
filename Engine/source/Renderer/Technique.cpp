#include "Technique.h"

namespace Renderer
{
	void Technique::Submit(const Drawable& drawable, size_t channelFilter) const noexcept(!IS_DEBUG)
	{
		if (active && ((channels & channelFilter) != 0))
		{
			for (const auto& step : steps)
			{
				step.Submit(drawable);
			}
		}
	}
	void Technique::InitializeParentReferences(const Drawable& parent) noexcept(!IS_DEBUG)
	{
		for (auto& s : steps)
		{
			s.InitializeParentReferences(parent);
		}
	}

	Technique::Technique(size_t channels)
		:
		channels{ channels }
	{}

	Technique::Technique(std::string name, size_t channels, bool startActive) noexcept(!IS_DEBUG)
		:
		active(startActive),
		name(name),
		channels(channels)
	{}
	void Technique::AddStep(Step step) noexcept(!IS_DEBUG)
	{
		steps.push_back(std::move(step));
	}
	bool Technique::IsActive() const noexcept(!IS_DEBUG)
	{
		return active;
	}
	void Technique::SetActiveState(bool active_in) noexcept(!IS_DEBUG)
	{
		active = active_in;
	}
	void Technique::Accept(TechniqueProbe& probe)
	{
		probe.SetTechnique(this);
		for (auto& s : steps)
		{
			s.Accept(probe);
		}
	}
	const std::string& Technique::GetName() const noexcept(!IS_DEBUG)
	{
		return name;
	}
	void Technique::Link(RenderGraph& rg)
	{
		for (auto& step : steps)
		{
			step.Link(rg);
		}
	}
}