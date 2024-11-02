#include "DepthStencil.h"

namespace Renderer
{
	DepthStencil::DepthStencil(D3D12RHI& gfx, UINT width, UINT height)
	{
        // Describe and create a DSV descriptor heap.
        {
            D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
            dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            dsvHeapDesc.NumDescriptors = 1;
            GetDevice(gfx)->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));
        }

        // Create Depth Buffer - Depth Stensil View (DSV).
        {
            CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
            CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
                DXGI_FORMAT_D24_UNORM_S8_UINT,
                width, height,
                1, 0, 1, 0,
                D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            clearValue.DepthStencil = { 1.0f, 0xFF };

            GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                &clearValue,
                IID_PPV_ARGS(&m_depthBuffer));

            m_depthStensilViewHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

            GetDevice(gfx)->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, m_depthStensilViewHandle);
        }
	}
    void DepthStencil::Clear(D3D12RHI& gfx) const noexcept
    {
        GetCommandList(gfx)->ClearDepthStencilView(m_depthStensilViewHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0xFF, 0, nullptr);
    }
	void DepthStencil::BindAsDepthStencil(D3D12RHI& gfx) const noexcept
	{
		GetCommandList(gfx)->OMSetRenderTargets(0, nullptr, FALSE, &m_depthStensilViewHandle);
	}
}