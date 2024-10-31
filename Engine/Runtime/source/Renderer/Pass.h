#pragma once
#include "D3D12RHI.h"
#include "Job.h"
#include <vector>

namespace Renderer
{
	class Pass
	{
	public:
		void Accept(Job job) noexcept
		{
			jobs.push_back(job);
		}
		void Execute(D3D12RHI& gfx) const noexcept
		{
			for (const auto& j : jobs)
			{
				j.Execute(gfx);
			}
		}
		void Reset() noexcept
		{
			jobs.clear();
		}
	private:
		std::vector<Job> jobs;
	};
}