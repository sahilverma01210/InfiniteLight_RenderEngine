#include "StepLinkingProbe.h"
#include "RenderGraph.h"
#include "Step.h"

namespace Renderer
{
	void StepLinkingProbe::OnSetStep()
	{
		pStep->Link(rg);
	}
}