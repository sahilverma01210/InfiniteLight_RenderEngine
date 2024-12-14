#pragma once
#include "BindableCodex.h"

namespace Renderer
{
	class Viewport : public Bindable
	{
	public:
		Viewport(D3D12RHI& gfx);
		Viewport(D3D12RHI& gfx, float width, float height);
		~Viewport() = default;
		void Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;

	private:
		D3D12_VIEWPORT m_viewPort{};
	};
}