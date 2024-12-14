#include "StepLinkingProbe.h"

namespace Renderer
{
	void StepLinkingProbe::OnSetStep()
	{
		m_pStep->Link(m_renderGraph);
	}
}