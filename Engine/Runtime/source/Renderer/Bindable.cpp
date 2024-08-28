#include "Bindable.h"

namespace Renderer
{
	ID3D12Device* Bindable::GetDevice(D3D12RHI& gfx) noexcept
	{
		return gfx.m_device.Get();
	}
	ID3D12CommandQueue* Bindable::GetCommandQueue(D3D12RHI& gfx) noexcept
	{
		return gfx.m_commandQueue.Get();
	}
	ID3D12CommandAllocator* Bindable::GetCommandAllocator(D3D12RHI& gfx) noexcept
	{
		return gfx.m_commandAllocator.Get();
	}
	ID3D12GraphicsCommandList* Bindable::GetCommandList(D3D12RHI& gfx) noexcept
	{
		return gfx.m_commandList.Get();
	}
	void Bindable::InsertFence(D3D12RHI& gfx) noexcept
	{
		gfx.InsertFence();
	}
}