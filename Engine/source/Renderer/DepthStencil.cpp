#include "DepthStencil.h"
#include "RenderTarget.h"
#include "D3D12RHI.h"

namespace Renderer
{
    DXGI_FORMAT MapUsageTypeless(DepthStencil::Usage usage)
    {
        switch (usage)
        {
        case DepthStencil::Usage::DepthStencil:
            return DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;
        case DepthStencil::Usage::ShadowDepth:
            return DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
        }
        throw std::runtime_error{ "Base usage for Typeless format map in DepthStencil." };
    }
    DXGI_FORMAT MapUsageTyped(DepthStencil::Usage usage)
    {
        switch (usage)
        {
        case DepthStencil::Usage::DepthStencil:
            return DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
        case DepthStencil::Usage::ShadowDepth:
            return DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
        }
        throw std::runtime_error{ "Base usage for Typed format map in DepthStencil." };
    }
    DXGI_FORMAT MapUsageColored(DepthStencil::Usage usage)
    {
        switch (usage)
        {
        case DepthStencil::Usage::DepthStencil:
            return DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        case DepthStencil::Usage::ShadowDepth:
            return DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
        }
        throw std::runtime_error{ "Base usage for Colored format map in DepthStencil." };
    }

    DepthStencil::DepthStencil(D3D12RHI& gfx)
        :
        DepthStencil(gfx, gfx.GetWidth(), gfx.GetHeight(), Usage::DepthStencil)
    {
    }

    DepthStencil::DepthStencil(D3D12RHI& gfx, UINT width, UINT height, Usage usage)
        :
        width(width),
        height(height)
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
                MapUsageTyped(usage),
                width, height,
                1, 0, 1, 0,
                D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = MapUsageTyped(usage);
            clearValue.DepthStencil = { 1.0f, 0xFF };

            GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                &clearValue,
                IID_PPV_ARGS(&m_depthBuffer));

            m_depthStensilViewHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

            D3D12_DEPTH_STENCIL_VIEW_DESC descView = {};
            descView.Format = MapUsageTyped(usage);
            descView.Flags = D3D12_DSV_FLAG_NONE;
            descView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            descView.Texture2D.MipSlice = 0;
            GetDevice(gfx)->CreateDepthStencilView(m_depthBuffer.Get(), &descView, m_depthStensilViewHandle);
        }
	}

    void DepthStencil::BindAsBuffer(D3D12RHI& gfx) noexcept
    {
        TransitionTo(gfx, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

        GetCommandList(gfx)->OMSetRenderTargets(0, nullptr, FALSE, &m_depthStensilViewHandle);
    }

    void DepthStencil::BindAsBuffer(D3D12RHI& gfx, BufferResource* renderTarget) noexcept
    {
        assert(dynamic_cast<RenderTarget*>(renderTarget) != nullptr);
        BindAsBuffer(gfx, static_cast<RenderTarget*>(renderTarget));
    }

    void DepthStencil::BindAsBuffer(D3D12RHI& gfx, RenderTarget* rt) noexcept
    {
        TransitionTo(gfx, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

        rt->BindAsBuffer(gfx, this);
    }

    void DepthStencil::Clear(D3D12RHI& gfx) noexcept
    {
        GetCommandList(gfx)->ClearDepthStencilView(m_depthStensilViewHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0xFF, 0, nullptr);
    }

    void DepthStencil::TransitionTo(D3D12RHI& gfx, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) const noexcept
    {
        auto resourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_depthBuffer.Get(), beforeState, afterState);
        GetCommandList(gfx)->ResourceBarrier(1, &resourceBarrier2);
    }

    unsigned int DepthStencil::GetWidth() const
    {
        return width;
    }
    unsigned int DepthStencil::GetHeight() const
    {
        return height;
    }
}