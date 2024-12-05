#pragma once
#include "TechniqueProbe.h"
#include "RenderGraph.h"
#include "Step.h"

namespace Renderer
{
	class StepLinkingProbe : public TechniqueProbe
	{
	protected:
		void OnSetStep() override;

	private:
		RenderGraph& rg;
	};
}