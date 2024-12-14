#include "RenderQueuePass.h"

namespace Renderer
{
	void RenderQueuePass::Accept(Job job) noexcept(!IS_DEBUG)
	{
		m_jobs.push_back(job);
	}

	void RenderQueuePass::Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		BindAll(gfx);

		for (const auto& job : m_jobs)
		{
			job.Execute(gfx);
		}
	}

	void RenderQueuePass::Reset() noexcept(!IS_DEBUG)
	{
		m_jobs.clear();
	}
}