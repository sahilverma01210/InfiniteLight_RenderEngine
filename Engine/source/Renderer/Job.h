#pragma once
#include "Step.h"
#include "Drawable.h"

namespace Renderer
{
	class Job
	{
	public:
		Job(const Step* pStep, const Drawable* pDrawable);
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG);

	private:
		const class Drawable* pDrawable;
		const class Step* pStep;
	};
}