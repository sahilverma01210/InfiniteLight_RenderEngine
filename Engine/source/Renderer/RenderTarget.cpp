#include "RenderTarget.h"

namespace Renderer
{
	RenderTarget::RenderTarget(D3D12RHI& gfx, UINT width, UINT height)
        :
        m_width(width),
        m_height(height)
	{
        INFOMAN(gfx);

        auto const heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        const D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT,
            static_cast<UINT64>(width),
            static_cast<UINT>(height),
            1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

        {
            D3D12_CLEAR_VALUE clearValue = { DXGI_FORMAT_R16G16B16A16_FLOAT, {0.0f, 0.0f, 0.0f, 0.0f} };
            D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
                &desc,
                D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue,
                IID_PPV_ARGS(&m_texureBuffer)
            ));
        }

        // Describe and create a RTV descriptor heap.
        {
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = 1;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            rtvHeapDesc.NodeMask = 1;
            D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
        }

        // Create Frame Resources - Render Target View (RTV).
        {
            UINT m_rtvDescriptorSize = GetDevice(gfx)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

            m_renderTargetViewHandle = rtvHandle;
            rtvHandle.ptr += m_rtvDescriptorSize;

            D3D12RHI_THROW_INFO_ONLY(GetDevice(gfx)->CreateRenderTargetView(m_texureBuffer.Get(), nullptr, m_renderTargetViewHandle));
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
	}

    RenderTarget::RenderTarget(D3D12RHI& gfx, ID3D12Resource* pTexture, std::optional<UINT> face)
    {
        INFOMAN(gfx);

        // Describe and create a RTV descriptor heap.
        {
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = 1;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            rtvHeapDesc.NodeMask = 1;
            D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
        }

        // Create Frame Resources - Render Target View (RTV).
        {
            // create the target view on the texture
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = pTexture->GetDesc().Format;
            if (face.has_value())
            {
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                rtvDesc.Texture2DArray.ArraySize = 1;
                rtvDesc.Texture2DArray.FirstArraySlice = *face;
                rtvDesc.Texture2DArray.MipSlice = 0;
            }
            else
            {
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D = D3D12_TEX2D_RTV{ 0 };
            }

            UINT m_rtvDescriptorSize = GetDevice(gfx)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

            m_renderTargetViewHandle = rtvHandle;
            rtvHandle.ptr += m_rtvDescriptorSize;

            D3D12RHI_THROW_INFO_ONLY(GetDevice(gfx)->CreateRenderTargetView(pTexture, &rtvDesc, m_renderTargetViewHandle));
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

	void RenderTarget::BindAsBuffer(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
        D3D12_CPU_DESCRIPTOR_HANDLE* null = nullptr;
        BindAsBuffer(gfx, null);
	}

    void RenderTarget::BindAsBuffer(D3D12RHI& gfx, BufferResource* depthStencil) noexcept(!IS_DEBUG)
    {
        assert(dynamic_cast<DepthStencil*>(depthStencil) != nullptr);
        BindAsBuffer(gfx, &(static_cast<DepthStencil*>(depthStencil)->m_depthStensilViewHandle));
    }

	void RenderTarget::BindAsBuffer(D3D12RHI& gfx, DepthStencil* depthStencil) noexcept(!IS_DEBUG)
	{
        BindAsBuffer(gfx, depthStencil ? &depthStencil->m_depthStensilViewHandle : nullptr);
	}

    void RenderTarget::BindAsBuffer(D3D12RHI& gfx, D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilView) noexcept(!IS_DEBUG)
    {
        INFOMAN_NOHR(gfx);
        D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->OMSetRenderTargets(1, &m_renderTargetViewHandle, FALSE, pDepthStencilView));
        gfx.SetRenderTargetBuffer(m_texureBuffer.Get());
    }

    void RenderTarget::Clear(D3D12RHI& gfx) noexcept(!IS_DEBUG)
    {
        const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        INFOMAN_NOHR(gfx);
        D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->ClearRenderTargetView(m_renderTargetViewHandle, clear_color_with_alpha, 0, nullptr));
    }

    void RenderTarget::Clear(D3D12RHI& gfx, const std::array<float, 4>& color) const noexcept(!IS_DEBUG)
    {
        INFOMAN_NOHR(gfx);
        D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->ClearRenderTargetView(m_renderTargetViewHandle, color.data(), 0, nullptr));
    }

    ID3D12Resource* RenderTarget::GetBuffer() const noexcept(!IS_DEBUG)
    {
        return m_texureBuffer.Get();
    }

    void RenderTarget::ResizeFrame(UINT width, UINT height)
    {
        m_width = width;
        m_height = height;
    }

    UINT RenderTarget::GetWidth() const noexcept(!IS_DEBUG)
    {
        return m_width;
    }

    UINT RenderTarget::GetHeight() const noexcept(!IS_DEBUG)
    {
        return m_height;
    }
}