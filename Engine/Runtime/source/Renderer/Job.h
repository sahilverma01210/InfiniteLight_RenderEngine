#pragma once

namespace Renderer
{
	class Job
	{
	public:
		Job(const class Step* pStep, const class Drawable* pDrawable);
		void Execute(class D3D12RHI& gfx) const noexcept;
	private:
		const class Drawable* pDrawable;
		const class Step* pStep;
	};
}