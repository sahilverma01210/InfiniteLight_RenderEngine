#pragma once

#include "D3D12RHI.h"
#include "Bindable.h"

namespace Renderer
{
	class Drawable
	{
	public:
		Drawable() = default;
		Drawable(const Drawable&) = delete;
		virtual XMMATRIX GetTransformXM() const noexcept = 0;
		void Draw(D3D12RHI& gfx) const;
		virtual void Update(float dt) noexcept = 0;
		void AddBindable(std::unique_ptr<Bindable> bindable) noexcept;
		virtual ~Drawable() = default;
	protected:
		UINT m_numIndices;
	private:
		std::vector<std::unique_ptr<Bindable>> bindables;
	};
}