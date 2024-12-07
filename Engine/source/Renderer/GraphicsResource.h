#pragma once
#include "D3D12RHI.h"

namespace Renderer
{
	class GraphicsResource
	{
	protected:
		static UINT GetWidth(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		static UINT GetHeight(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		static ID3D12Device* GetDevice(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		static ID3D12CommandQueue* GetCommandQueue(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		static ID3D12CommandAllocator* GetCommandAllocator(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		static ID3D12GraphicsCommandList* GetCommandList(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		static IDXGISwapChain3* GetSwapChain(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		static std::vector<ComPtr<ID3D12Resource>>& GetBackBuffers(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		static void InsertFence(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		static DxgiInfoManager& GetInfoManager(D3D12RHI& gfx);
	};
}