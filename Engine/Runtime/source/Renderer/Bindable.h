#pragma once
#include "D3D12RHI.h"

namespace Renderer
{
	class Bindable
	{
	public:
		virtual ~Bindable() = default;
		virtual void Update(D3D12RHI& gfx, const void* pData) noexcept = 0;
		virtual void Bind(D3D12RHI& gfx) noexcept = 0;
		virtual std::string GetUID() const noexcept
		{
			assert(false);
			return "";
		}
	protected:
		static ID3D12Device* GetDevice(D3D12RHI& gfx) noexcept;
		static ID3D12CommandQueue* GetCommandQueue(D3D12RHI& gfx) noexcept;
		static ID3D12CommandAllocator* GetCommandAllocator(D3D12RHI& gfx) noexcept;
		static ID3D12GraphicsCommandList* GetCommandList(D3D12RHI& gfx) noexcept;
		static void InsertFence(D3D12RHI& gfx) noexcept;
	};
}