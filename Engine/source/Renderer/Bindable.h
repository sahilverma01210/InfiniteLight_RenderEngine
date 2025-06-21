#pragma once
#include "GraphicsResource.h"
//#include "DynamicConstant.h"

namespace Renderer
{
	class Drawable;
	class TechniqueProbe;

	class Bindable : public GraphicsResource
	{
	public:
		virtual ~Bindable() = default;
		virtual void Update(D3D12RHI& gfx, const void* pData) noexcept(!IS_DEBUG) {}
		virtual void Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG) {}
		virtual void Accept(TechniqueProbe&) {}
	};
}