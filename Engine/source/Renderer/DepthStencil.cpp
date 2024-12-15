#include "DepthStencil.h"
// DepthStencil objects are linked before RenderTarget object. Hence we cannot include RenderTarget.h in DepthStencil.h
#include "RenderTarget.h"

namespace Renderer
{
    DepthStencil::DepthStencil(D3D12RHI& gfx)
        :
        DepthStencil(gfx, gfx.GetWidth(), gfx.GetHeight(), DepthUsage::DepthStencil)
    {
    }

    DepthStencil::DepthStencil(D3D12RHI& gfx, ID3D12Resource* depthBuffer, UINT face)
        :
        m_depthBuffer(depthBuffer),
        m_width(depthBuffer->GetDesc().Width),
        m_height(depthBuffer->GetDesc().Height)
    {
        INFOMAN(gfx);

        // Describe and create a DSV descriptor heap.
        {
            D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
            dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            dsvHeapDesc.NumDescriptors = 1;
            D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
        }

        // Create Depth Buffer - Depth Stensil View (DSV).
        {
            m_depthStensilViewHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

            D3D12_DEPTH_STENCIL_VIEW_DESC descView = {};
            descView.Format = DXGI_FORMAT_D32_FLOAT;
            descView.Flags = D3D12_DSV_FLAG_NONE;
            descView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            descView.Texture2DArray.MipSlice = 0;
            descView.Texture2DArray.ArraySize = 1;
            descView.Texture2DArray.FirstArraySlice = face;
            D3D12RHI_THROW_INFO_ONLY(GetDevice(gfx)->CreateDepthStencilView(m_depthBuffer.Get(), &descView, m_depthStensilViewHandle));
        }
    }

    DepthStencil::DepthStencil(D3D12RHI& gfx, UINT width, UINT height, DepthUsage usage)
        :
        m_width(width),
        m_height(height)
	{
        INFOMAN(gfx);

        // Describe and create a DSV descriptor heap.
        {
            D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
            dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            dsvHeapDesc.NumDescriptors = 1;
            D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
        }

        // Create Depth Buffer - Depth Stensil View (DSV).
        {
            CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
            CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
                gfx.MapUsageTypeless(usage),
                m_width, m_height,
                1, 0, 1, 0,
                D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = gfx.MapUsageTyped(usage);
            clearValue.DepthStencil = { 1.0f, 0xFF };

            D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                &clearValue,
                IID_PPV_ARGS(&m_depthBuffer)));

            m_depthStensilViewHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

            D3D12_DEPTH_STENCIL_VIEW_DESC descView = {};
            descView.Format = gfx.MapUsageTyped(usage);
            descView.Flags = D3D12_DSV_FLAG_NONE;
            descView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            descView.Texture2D.MipSlice = 0;
            D3D12RHI_THROW_INFO_ONLY(GetDevice(gfx)->CreateDepthStencilView(m_depthBuffer.Get(), &descView, m_depthStensilViewHandle));
        }
	}

    void DepthStencil::BindAsBuffer(D3D12RHI& gfx, BufferResource* bufferResource) noexcept(!IS_DEBUG)
    {
        if (bufferResource)
        {
            static_cast<RenderTarget*>(bufferResource)->BindAsBuffer(gfx, this);
        }
        else
        {
            INFOMAN_NOHR(gfx);
            D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->OMSetRenderTargets(0, nullptr, FALSE, &m_depthStensilViewHandle));
        }
    }

    void DepthStencil::Clear(D3D12RHI& gfx) noexcept(!IS_DEBUG)
    {
        INFOMAN_NOHR(gfx);
        D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->ClearDepthStencilView(m_depthStensilViewHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0xFF, 0, nullptr));
    }

    unsigned int DepthStencil::GetWidth() const
    {
        return m_width;
    }

    unsigned int DepthStencil::GetHeight() const
    {
        return m_height;
    }

    ID3D12Resource* DepthStencil::GetBuffer() const noexcept(!IS_DEBUG)
    {
        return m_depthBuffer.Get();
    }
}