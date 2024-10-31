#pragma once
#include <array>
#include "CommonBindables.h"
#include "D3D12RHI.h"
#include "Job.h"
#include "Pass.h"
#include "PerfLog.h"

namespace Renderer
{
	class FrameCommander
	{
	public:
		void Accept(Job job, size_t target) noexcept
		{
			passes[target].Accept(job);
		}
		void Execute(D3D12RHI& gfx) const noexcept
		{
			//PerfLog::Start("Begin");
			passes[0].Execute(gfx);
			passes[1].Execute(gfx);
			passes[2].Execute(gfx);
			//PerfLog::Mark("Resolve 2x");
		}
		void Reset() noexcept
		{
			for (auto& p : passes)
			{
				p.Reset();
			}
		}
	private:
		std::array<Pass, 3> passes;
	};
}