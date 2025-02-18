#include "Job.h"

namespace Renderer
{
	Job::Job(const Step* pStep, const Drawable* pDrawable)
		:
		m_pDrawable{ pDrawable },
		m_pStep{ pStep }
	{}

	void Job::Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		m_pDrawable->Bind(gfx, m_pStep->GetTargetPass());
		m_pStep->Bind(gfx);
		m_pDrawable->Draw(gfx);
	}
}