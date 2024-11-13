#pragma once

namespace Renderer
{
	class D3D12RHI;

	class BufferResource
	{
	public:
		virtual ~BufferResource() = default;
		virtual void BindAsBuffer(D3D12RHI&) noexcept = 0;
		virtual void BindAsBuffer(D3D12RHI&, BufferResource*) noexcept = 0;
		virtual void Clear(D3D12RHI&) noexcept = 0;
	};
}