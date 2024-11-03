#pragma once
#include "GraphicsResource.h"
#include "DynamicConstant.h"

namespace Renderer
{
	class Drawable;
	class TechniqueProbe;

	class Bindable : public GraphicsResource
	{
	public:
		virtual ~Bindable() = default;
		virtual void Update(D3D12RHI& gfx, const void* pData) noexcept {}
		virtual void Update(D3D12RHI& gfx, Buffer dataBuffer) noexcept {}
		virtual void Bind(D3D12RHI& gfx) noexcept {}
		virtual void InitializeParentReference(const Drawable&) noexcept {}
		virtual void Accept(TechniqueProbe&) {}
		virtual std::string GetUID() const noexcept
		{
			assert(false);
			return "";
		}
	};

	class CloningBindable : public Bindable
	{
	public:
		virtual std::unique_ptr<CloningBindable> Clone() const noexcept = 0;
	};
}