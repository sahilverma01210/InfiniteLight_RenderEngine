#pragma once
#include "TechniqueProbe.h"

namespace Renderer
{
	class RenderGraph;

	class StepLinkingProbe : public TechniqueProbe
	{
	protected:
		void OnSetStep() override;
	private:
		RenderGraph& rg;
	};
}