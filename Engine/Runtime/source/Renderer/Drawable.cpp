#include "Drawable.h"

namespace Renderer
{
	std::vector<std::shared_ptr<Bindable>> Drawable::staticBinds;

	void Drawable::Draw(D3D12RHI& gfx, XMMATRIX transform) const
	{
		gfx.SetTransform(transform);

		psoBindable->Bind(gfx);

		for (auto& bindable : bindables)
		{
			bindable->Bind(gfx);
		}

		for (auto& b : staticBinds)
		{
			b->Bind(gfx);
		}

		gfx.DrawIndexed(GetNumIndices());
	}

	void Drawable::AddPipelineStateObject(std::unique_ptr<Bindable> bindable) noexcept
	{
		psoBindable = std::move(bindable);
	}

	void Drawable::AddBindable(std::shared_ptr<Bindable> bindable) noexcept
	{
		bindables.push_back(std::move(bindable));
	}
}