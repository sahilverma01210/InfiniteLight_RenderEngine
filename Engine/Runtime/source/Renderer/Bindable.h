#pragma once

#include "D3D12RHI.h"

namespace Renderer
{
	class Bindable
	{
	public:
		virtual ~Bindable() = default;
		virtual void Bind(D3D12RHI& gfx) noexcept = 0;
	protected:
		static ID3D12Device* GetDevice(D3D12RHI& gfx) noexcept;
		static ID3D12CommandQueue* GetCommandQueue(D3D12RHI& gfx) noexcept;
		static ID3D12CommandAllocator* GetCommandAllocator(D3D12RHI& gfx) noexcept;
		static ID3D12GraphicsCommandList* GetCommandList(D3D12RHI& gfx) noexcept;
		static ID3D12DescriptorHeap* GetSRVDescriptorHeap(D3D12RHI& gfx) noexcept;
		static void InsertFence(D3D12RHI& gfx) noexcept;
	};
}