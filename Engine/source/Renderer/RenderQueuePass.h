#pragma once
#include "BindingPass.h"
#include "Job.h"

namespace Renderer
{
	class RenderQueuePass : public BindingPass
	{
		using BindingPass::BindingPass;
	public:
		void Accept(Job job) noexcept(!IS_DEBUG);
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override;
		void Reset() noexcept(!IS_DEBUG) override;

	private:
		std::vector<Job> jobs;
	};
}