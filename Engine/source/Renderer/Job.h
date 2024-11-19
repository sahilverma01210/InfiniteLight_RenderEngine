#pragma once

namespace Renderer
{
	class Drawable;
	class D3D12RHI;
	class Step;

	class Job
	{
	public:
		Job(const Step* pStep, const Drawable* pDrawable);
		void Execute(D3D12RHI& gfx) const noexcept;
	private:
		const class Drawable* pDrawable;
		const class Step* pStep;
	};
}