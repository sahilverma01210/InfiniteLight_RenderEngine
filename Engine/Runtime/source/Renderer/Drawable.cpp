#include "Drawable.h"

namespace Renderer
{
	std::vector<std::shared_ptr<Bindable>> Drawable::staticBinds;

	void Drawable::Draw(D3D12RHI& gfx, XMMATRIX transform) const
	{
		gfx.SetTransform(transform);

		rootSignBindable->Bind(gfx);
		psoBindable->Bind(gfx);

		for (auto& bindable : bindables)
		{
			bindable->Bind(gfx);
		}

		for (auto& b : staticBinds)
		{
			b->Bind(gfx);
		}

		if (srvBindable != nullptr) srvBindable->Bind(gfx);

		gfx.DrawIndexed(GetNumIndices());
	}

	void Drawable::AddRootSignatureObject(std::unique_ptr<Bindable> bindable) noexcept
	{
		rootSignBindable = std::move(bindable);
	}

	void Drawable::AddPipelineStateObject(std::unique_ptr<Bindable> bindable) noexcept
	{
		psoBindable = std::move(bindable);
	}

	void Drawable::AddShaderResourceViewObject(std::unique_ptr<Bindable> bindable) noexcept
	{
		srvBindable = std::move(bindable);
	}

	void Drawable::AddBindable(std::shared_ptr<Bindable> bindable) noexcept
	{
		bindables.push_back(std::move(bindable));
	}
}