#pragma once
#include "D3D12RHI.h"

namespace Renderer
{
	class BufferResource
	{
	public:
		virtual ~BufferResource() = default;
		virtual void BindAsBuffer(D3D12RHI&) noexcept(!IS_DEBUG) = 0;
		virtual void BindAsBuffer(D3D12RHI&, BufferResource*) noexcept(!IS_DEBUG) = 0;
		virtual void Clear(D3D12RHI&) noexcept(!IS_DEBUG) = 0;
	};
}