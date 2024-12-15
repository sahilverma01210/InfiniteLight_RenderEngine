#include "BindingPass.h"

namespace Renderer
{
	BindingPass::BindingPass(std::string name, std::vector<std::shared_ptr<Bindable>> binds)
		:
		Pass(std::move(name)),
		m_binds(std::move(binds))
	{}

	void BindingPass::AddBind(std::shared_ptr<Bindable> bind) noexcept(!IS_DEBUG)
	{
		m_binds.push_back(std::move(bind));
	}

	void BindingPass::BindAll(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		BindBufferResources(gfx);
		for (auto& bind : m_binds)
		{
			bind->Bind(gfx);
		}
	}

	void BindingPass::Finalize()
	{
		Pass::Finalize();
		if (!m_renderTargetVector.size() && !m_depthStencil)
		{
			throw RG_EXCEPTION("BindingPass [" + GetName() + "] needs at least one of a renderTarget or depthStencil");
		}
	}

	void BindingPass::BindBufferResources(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
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