#pragma once
#include "GraphicsResource.h"

namespace Renderer
{
	class Drawable;
	class TechniqueProbe;

	class Bindable : public GraphicsResource
	{
	public:
		virtual ~Bindable() = default;
		virtual void Update(D3D12RHI& gfx, const void* pData) noexcept = 0;
		virtual void Bind(D3D12RHI& gfx) noexcept = 0;
		virtual void InitializeParentReference(const Drawable&) noexcept
		{}
		virtual void Accept(TechniqueProbe&)
		{}
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