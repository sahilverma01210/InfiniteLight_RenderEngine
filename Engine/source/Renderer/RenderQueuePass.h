#pragma once
#include "BindingPass.h"
#include "Job.h"

namespace Renderer
{
	class RenderQueuePass : public BindingPass
	{
		using BindingPass::BindingPass;
	public:
		void Accept(Job job) noexcept;
		void Execute(D3D12RHI& gfx) const noexcept override;
		void Reset() noexcept override;

	private:
		std::vector<Job> jobs;
	};
}