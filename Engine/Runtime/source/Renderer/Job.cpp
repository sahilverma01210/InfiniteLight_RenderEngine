#include "Job.h"
#include "Step.h"
#include "Drawable.h"

namespace Renderer
{
	Job::Job(const Step* pStep, const Drawable* pDrawable)
		:
		pDrawable{ pDrawable },
		pStep{ pStep }
	{}
	void Job::Execute(D3D12RHI& gfx) const noexcept
	{
		gfx.SetTransform(pDrawable->GetTransformXM());
		pDrawable->Bind(gfx, pStep->GetTargetPass());
		pStep->Bind(gfx);
		if (pStep->GetTargetPass() == 0) pDrawable->BindLighting(gfx); // Only Fist Added Step or Pass uses Lighting data.
		gfx.DrawIndexed(pDrawable->GetIndexCount());
	}
}