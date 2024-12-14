#include "Technique.h"

namespace Renderer
{
	void Technique::Submit(const Drawable& drawable, size_t channelFilter) const noexcept(!IS_DEBUG)
	{
		if (m_active && ((m_channels & channelFilter) != 0))
		{
			for (const auto& step : m_steps)
			{
				step.Submit(drawable);
			}
		}
	}

	Technique::Technique(size_t channels)
		:
		m_channels{ channels }
	{}

	Technique::Technique(std::string name, size_t channels, bool startActive) noexcept(!IS_DEBUG)
		:
		m_active(startActive),
		m_name(name),
		m_channels(channels)
	{}
	void Technique::AddStep(Step step) noexcept(!IS_DEBUG)
	{
		m_steps.push_back(std::move(step));
	}
	bool Technique::IsActive() const noexcept(!IS_DEBUG)
	{
		return m_active;
	}
	void Technique::SetActiveState(bool active_in) noexcept(!IS_DEBUG)
	{
		m_active = active_in;
	}
	void Technique::Accept(TechniqueProbe& probe)
	{
		probe.SetTechnique(this);
		for (auto& s : m_steps)
		{
			s.Accept(probe);
		}
	}
	const std::string& Technique::GetName() const noexcept(!IS_DEBUG)
	{
		return m_name;
	}
	void Technique::Link(RenderGraph& rg)
	{
		for (auto& step : m_steps)
		{
			step.Link(rg);
		}
	}
}