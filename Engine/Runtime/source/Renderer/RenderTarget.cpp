#include "RenderTarget.h"
#include "DepthStencil.h"

namespace Renderer
{
	RenderTarget::RenderTarget(D3D12RHI& gfx, UINT width, UINT height)
	{
        auto const heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        const D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT,
            static_cast<UINT64>(width),
            static_cast<UINT>(height),
            1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

        D3D12_CLEAR_VALUE clearValue = { DXGI_FORMAT_R16G16B16A16_FLOAT, {0.0f, 0.0f, 0.0f, 0.0f} };

        GetDevice(gfx)->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
            &desc,
            D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue,
            IID_PPV_ARGS(&m_texureBuffer));

        UINT backBufferCount = GetBackBuffers(gfx).size();

        // Describe and create a RTV descriptor heap.
        {
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = backBufferCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            rtvHeapDesc.NodeMask = 1;
            GetDevice(gfx)->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
        }

        // Create Frame Resources - Render Target View (RTV).
        {
            UINT m_rtvDescriptorSize = GetDevice(gfx)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

            m_renderTargetViewHandle = rtvHandle;
            rtvHandle.ptr += m_rtvDescriptorSize;

            GetDevice(gfx)->CreateRenderTargetView(m_texureBuffer.Get(), nullptr, m_renderTargetViewHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
	}
    ID3D12Resource* RenderTarget::GetBuffer() const noexcept
    {
        return m_texureBuffer.Get();
    }
    void RenderTarget::TransitionTo(D3D12RHI& gfx, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) const noexcept
    {
        auto resourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_texureBuffer.Get(), beforeState, afterState);
        GetCommandList(gfx)->ResourceBarrier(1, &resourceBarrier2);
    }
    void RenderTarget::Clear(D3D12RHI& gfx) const noexcept
    {
        const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        GetCommandList(gfx)->ClearRenderTargetView(m_renderTargetViewHandle, clear_color_with_alpha, 0, nullptr);
    }
    void RenderTarget::Clear(D3D12RHI& gfx, const std::array<float, 4>& color) const noexcept
    {
        GetCommandList(gfx)->ClearRenderTargetView(m_renderTargetViewHandle, color.data(), 0, nullptr);
    }
	void RenderTarget::BindAsTexture(D3D12RHI& gfx, UINT slot) const noexcept
	{
	}
	void RenderTarget::BindAsTarget(D3D12RHI& gfx) const noexcept
	{
		GetCommandList(gfx)->OMSetRenderTargets(1, &m_renderTargetViewHandle, FALSE, nullptr);
	}
	void RenderTarget::BindAsTarget(D3D12RHI& gfx, const DepthStencil& depthStencil) const noexcept
	{
		GetCommandList(gfx)->OMSetRenderTargets(1, &m_renderTargetViewHandle, FALSE, &depthStencil.m_depthStensilViewHandle);
	}
}