#pragma once
#include "BindingPass.h"
#include "Job.h"

namespace Renderer
{
	class RenderQueuePass : public BindingPass
	{
	public:
		using BindingPass::BindingPass;
		void Accept(Job job) noexcept;
		void Execute(D3D12RHI& gfx) const noexcept override;
		void Reset() noexcept override;
	private:
		std::vector<Job> jobs;
	};
}