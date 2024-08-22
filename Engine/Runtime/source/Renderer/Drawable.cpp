#include "Drawable.h"

namespace Renderer
{
	void Drawable::Draw(D3D12RHI& gfx) const
	{
		for (auto& bindable : bindables)
		{
			bindable->Bind(gfx);
		}

		gfx.DrawIndexed(m_numIndices);
	}

	void Drawable::AddBindable(std::unique_ptr<Bindable> bindable) noexcept
	{
		bindables.push_back(std::move(bindable));
	}
}