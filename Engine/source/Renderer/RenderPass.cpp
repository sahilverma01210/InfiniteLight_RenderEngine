#include "RenderPass.h"

namespace Renderer
{
	RenderPass::RenderPass(std::string name)
		:
		Pass(std::move(name))
	{
	}

	void RenderPass::Accept(Job job) noexcept(!IS_DEBUG)
	{
		m_jobs.push_back(job);
	}

	void RenderPass::Finalize()
	{
		Pass::Finalize();
		if (!m_renderTargetVector.size() && !m_depthStencil)
		{
			throw RG_EXCEPTION("Render Pass [" + GetName() + "] needs at least one of a renderTarget or depthStencil");
		}
	}

	void RenderPass::Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		BindBufferResources(gfx);

		for (const auto& job : m_jobs)
		{
			job.Execute(gfx);
		}
	}

	void RenderPass::Reset() noexcept(!IS_DEBUG)
	{
		m_jobs.clear();
	}

	void RenderPass::BindBufferResources(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		UINT renderSize = m_renderTargetVector.size();

		if (renderSize)
		{
			m_renderTargetVector[renderSize == 1 ? 0 : gfx.GetCurrentBackBufferIndex()]->BindAsBuffer(gfx, m_depthStencil.get());
		}
		else
		{
			m_depthStencil->BindAsBuffer(gfx, nullptr);
		}
	}
}