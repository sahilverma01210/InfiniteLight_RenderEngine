#include "GraphicsResource.h"

namespace Renderer
{
	ID3D12Device* GraphicsResource::GetDevice(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		return gfx.m_device.Get();
	}

	ID3D12CommandQueue* GraphicsResource::GetCommandQueue(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		return gfx.m_commandQueue.Get();
	}

	ID3D12CommandAllocator* GraphicsResource::GetCommandAllocator(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		return gfx.m_commandAllocator.Get();
	}

	ID3D12GraphicsCommandList* GraphicsResource::GetCommandList(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		return gfx.m_currentCommandList.Get();
	}

	IDXGISwapChain3* GraphicsResource::GetSwapChain(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		return gfx.m_swapChain.Get();
	}

	std::vector<ComPtr<ID3D12Resource>>& GraphicsResource::GetBackBuffers(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		return gfx.m_backBuffers;
	}

	DxgiInfoManager& GraphicsResource::GetInfoManager(D3D12RHI& gfx)
	{
#ifndef NDEBUG
		return gfx.m_infoManager;
#else
		throw std::logic_error("Tried to access gfx.infoManager in Release config");
#endif
	}
}