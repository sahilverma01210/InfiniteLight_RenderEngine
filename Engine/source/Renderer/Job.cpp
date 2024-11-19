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
		if (pStep->GetTargetPass() == "lambertian") pDrawable->BindLighting(gfx); // Only Lambertian Step or Pass uses Lighting data in Material/SolidSphere.
		gfx.DrawIndexed(pDrawable->GetIndexCount());
	}
}