#pragma once
#include "Step.h"
#include "Channels.h"
#include "Drawable.h"
#include "TechniqueProbe.h"

namespace Renderer
{
	class Technique
	{
	public:
		Technique(size_t channels);
		Technique(std::string name, size_t channels, bool startActive = true) noexcept(!IS_DEBUG);
		void Submit(const Drawable& drawable, size_t channels) const noexcept(!IS_DEBUG);
		void AddStep(Step step) noexcept(!IS_DEBUG);
		bool IsActive() const noexcept(!IS_DEBUG);
		void SetActiveState(bool active_in) noexcept(!IS_DEBUG);
		void Accept(TechniqueProbe& probe);
		const std::string& GetName() const noexcept(!IS_DEBUG);
		void Link(RenderGraph&);

	private:
		bool active = true;
		std::vector<Step> steps;
		std::string name;
		size_t channels;
	};
}