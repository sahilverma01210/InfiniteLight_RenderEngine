#include "RenderPass.h"

namespace Renderer
{
	RenderPass::RenderPass(std::string name)
		:
		Pass(std::move(name))
	{
	}

	void RenderPass::Accept(const Drawable& drawable) noexcept(!IS_DEBUG)
	{
		m_drawables.push_back(drawable);
	}

	void RenderPass::Finalize()
	{
		Pass::Finalize();
		if (!m_renderTargets.size() && !m_depthStencil)
		{
			throw RG_EXCEPTION("Render Pass [" + GetName() + "] needs at least one of a renderTarget or depthStencil");
		}
	}

	void RenderPass::Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		gfx.SetRenderTargets(m_renderTargets, m_depthStencil);
		gfx.SetGPUResources();

		for (const auto& drawable : m_drawables)
		{
			m_rootSignature->Bind(gfx);
			m_pipelineStateObject->Bind(gfx);

			drawable.get().Bind(gfx);
			drawable.get().Draw(gfx);
		}
	}

	void RenderPass::Reset() noexcept(!IS_DEBUG)
	{
		m_drawables.clear();
	}
}