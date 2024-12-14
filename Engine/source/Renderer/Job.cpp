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
		gfx.SetTransform(m_pDrawable->GetTransformXM());
		m_pDrawable->Bind(gfx, m_pStep->GetTargetPass());
		m_pStep->Bind(gfx);
		if (m_pStep->GetTargetPass() == "lambertian") m_pDrawable->BindLighting(gfx); // Only Lambertian Step or Pass uses Lighting data in Material/SolidSphere.
		gfx.DrawIndexed(m_pDrawable->GetIndexCount());
	}
}