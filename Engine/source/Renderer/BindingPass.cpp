#include "BindingPass.h"
#include "Bindable.h"
#include "RenderTarget.h"
#include "DepthStencil.h"
#include "RenderGraphCompileException.h"

namespace Renderer
{
	BindingPass::BindingPass(std::string name, std::vector<std::shared_ptr<Bindable>> binds)
		:
		Pass(std::move(name)),
		binds(std::move(binds))
	{}

	void BindingPass::AddBind(std::shared_ptr<Bindable> bind) noexcept
	{
		binds.push_back(std::move(bind));
	}

	void BindingPass::BindAll(D3D12RHI& gfx) const noexcept
	{
		BindBufferResources(gfx);
		for (auto& bind : binds)
		{
			bind->Bind(gfx);
		}
	}

	void BindingPass::Finalize()
	{
		Pass::Finalize();
		//if (!renderTarget && !depthStencil)
		//{
		//	throw RGC_EXCEPTION("BindingPass [" + GetName() + "] needs at least one of a renderTarget or depthStencil");
		//}
	}

	void BindingPass::BindBufferResources(D3D12RHI& gfx) const noexcept
	{
		UINT renderSize = renderTargetVector.size();

		if (renderSize)
		{
			renderTargetVector[renderSize == 1 ? 0 : gfx.GetCurrentBackBufferIndex()]->BindAsBuffer(gfx, depthStencil.get());
		}
		else
		{
			depthStencil->BindAsBuffer(gfx);
		}
	}
}