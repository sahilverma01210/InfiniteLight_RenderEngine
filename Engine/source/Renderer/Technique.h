#pragma once
#include "Step.h"
#include "Drawable.h"
#include "TechniqueProbe.h"

namespace Renderer
{
	struct Channel
	{
		static const size_t main = 0b1;
		static const size_t shadow = 0b10;
	};

	class Technique
	{
	public:
		Technique(size_t channels);
		Technique(std::string name, size_t channels, bool startActive = true) noexcept(!IS_DEBUG);
		const std::string& GetName() const noexcept(!IS_DEBUG);
		void SetActiveState(bool active_in) noexcept(!IS_DEBUG);
		bool GetActiveState() const noexcept(!IS_DEBUG);
		void AddStep(Step step) noexcept(!IS_DEBUG);
		void Link(RenderGraph&);
		void Submit(const Drawable& drawable, size_t channels) const noexcept(!IS_DEBUG);
		void Accept(TechniqueProbe& probe);

	private:
		bool m_active = true;
		std::string m_name;
		size_t m_channels;
		std::vector<Step> m_steps;
	};
}