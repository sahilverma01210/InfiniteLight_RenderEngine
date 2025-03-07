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
		if (!m_renderTarget && !m_depthStencil)
		{
			throw RG_EXCEPTION("Render Pass [" + GetName() + "] needs at least one of a renderTarget or depthStencil");
		}
	}

	void RenderPass::Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		BindRenderGraphResources(gfx);
		gfx.SetGPUResources();

		for (const auto& job : m_jobs)
		{
			m_rootSignature->Bind(gfx);
			m_pipelineStateObject->Bind(gfx);

			job.Execute(gfx);
		}
	}

	void RenderPass::Reset() noexcept(!IS_DEBUG)
	{
		m_jobs.clear();
	}

	void RenderPass::BindRenderGraphResources(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		if (!m_depthOnlyPass)
		{
			m_renderTarget->BindAsBuffer(gfx, m_depthStencil.get());
		}
		else
		{
			m_depthStencil->BindAsBuffer(gfx, nullptr);
		}
	}
}