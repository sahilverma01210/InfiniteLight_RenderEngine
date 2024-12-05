#include "StepLinkingProbe.h"

namespace Renderer
{
	void StepLinkingProbe::OnSetStep()
	{
		pStep->Link(rg);
	}
}