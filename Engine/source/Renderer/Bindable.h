#pragma once
#include "GraphicsResource.h"
#include "DynamicConstant.h"

namespace Renderer
{
	class Drawable;
	class TechniqueProbe;
	class D3D12RHI;

	class Bindable : public GraphicsResource
	{
	public:
		virtual ~Bindable() = default;
		virtual void Update(D3D12RHI& gfx, const void* pData) noexcept(!IS_DEBUG) {}
		virtual void Update(D3D12RHI& gfx, Buffer dataBuffer) noexcept(!IS_DEBUG) {}
		virtual void Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG) {}
		virtual void InitializeParentReference(const Drawable&) noexcept(!IS_DEBUG) {}
		virtual void Accept(TechniqueProbe&) {}
		virtual std::string GetUID() const noexcept(!IS_DEBUG)
		{
			assert(false);
			return "";
		}
	};
}