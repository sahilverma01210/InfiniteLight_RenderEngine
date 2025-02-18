#pragma once
#include "D3D12RHI.h"

namespace Renderer
{
	class RenderGraphResource
	{
	public:
		virtual ~RenderGraphResource() = default;
		virtual void BindAsBuffer(D3D12RHI&, RenderGraphResource*) noexcept(!IS_DEBUG) = 0;
		virtual void Clear(D3D12RHI&) noexcept(!IS_DEBUG) = 0;
	};
}