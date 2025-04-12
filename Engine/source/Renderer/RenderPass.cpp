#include "RenderPass.h"

namespace Renderer
{
	RenderPass::RenderPass(std::string name, RenderPassType type)
		:
		Pass(std::move(name)),
		m_renderPassType(type)
	{
	}

	void RenderPass::Accept(const Drawable& drawable) noexcept(!IS_DEBUG)
	{
		m_drawables.push_back(drawable);
	}

	void RenderPass::Finalize()
	{
		Pass::Finalize();
		if (m_renderPassType == RenderPassType::Graphics && !m_renderTargets.size() && !m_depthStencil)
		{
			throw RG_EXCEPTION("Render Pass [" + GetName() + "] needs at least one of a renderTarget or depthStencil");
		}
	}

	void RenderPass::Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		if (m_renderPassType == RenderPassType::Graphics)
		{
			gfx.SetGPUResources();

			m_rootSignature->Bind(gfx);
			m_pipelineStateObject->Bind(gfx);

			gfx.SetRenderTargets(m_renderTargets, m_depthStencil);

			for (const auto& drawable : m_drawables)
			{
				drawable.get().Bind(gfx);
				drawable.get().Draw(gfx);
			}
		}
		else if (m_renderPassType == RenderPassType::Compute)
		{
			gfx.Dispatch(DivideAndRoundUp(gfx.GetWidth(), 16), DivideAndRoundUp(gfx.GetHeight(), 16), 1);
		}
	}

	void RenderPass::Reset() noexcept(!IS_DEBUG)
	{
		m_drawables.clear();
	}
}