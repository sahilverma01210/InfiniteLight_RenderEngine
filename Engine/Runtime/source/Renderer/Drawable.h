#pragma once

#include "D3D12RHI.h"
#include "Bindable.h"

namespace Renderer
{
	class Drawable
	{
		template<class T>
		friend class Object;
	public:
		Drawable() = default;
		Drawable(const Drawable&) = delete;
		virtual XMMATRIX GetTransformXM() const noexcept = 0;
		void Draw(D3D12RHI& gfx, XMMATRIX transform) const;
		virtual void Update(float dt) noexcept {}
		void AddBindable(std::unique_ptr<Bindable> bindable) noexcept;
		virtual ~Drawable() = default;
	private:
		virtual const std::vector<std::unique_ptr<Bindable>>& GetStaticBinds() const noexcept = 0;
		virtual const UINT GetNumIndices() const noexcept = 0;
		std::vector<std::unique_ptr<Bindable>> bindables;
	};
}