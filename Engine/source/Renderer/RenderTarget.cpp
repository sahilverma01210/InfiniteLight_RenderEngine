#include "RenderTarget.h"

namespace Renderer
{
	RenderTarget::RenderTarget(D3D12RHI& gfx, UINT width, UINT height)
	{
        INFOMAN(gfx);

        auto const heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        const D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM,
            static_cast<UINT64>(width),
            static_cast<UINT>(height),
            1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

        {
            D3D12_CLEAR_VALUE clearValue = { DXGI_FORMAT_R8G8B8A8_UNORM, {0.0f, 0.0f, 0.0f, 0.0f} };
            D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
                &desc,
                D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue,
                IID_PPV_ARGS(&m_resourceBuffer)
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

            m_descHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
			m_gpuDescHandle = m_rtvHeap->GetGPUDescriptorHandleForHeapStart();

            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

            rtvHandle.ptr += m_rtvDescriptorSize;

            D3D12RHI_THROW_INFO_ONLY(GetDevice(gfx)->CreateRenderTargetView(m_resourceBuffer.Get(), nullptr, m_descHandle));
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
	}

    RenderTarget::RenderTarget(D3D12RHI& gfx, ID3D12Resource* pTexture, std::optional<UINT> face)
    {
        INFOMAN(gfx);

        m_resourceBuffer = pTexture;

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

            m_descHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
			m_gpuDescHandle = m_rtvHeap->GetGPUDescriptorHandleForHeapStart();

            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

            rtvHandle.ptr += m_rtvDescriptorSize;

            D3D12RHI_THROW_INFO_ONLY(GetDevice(gfx)->CreateRenderTargetView(pTexture, &rtvDesc, m_descHandle));
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    void RenderTarget::Clear(D3D12RHI& gfx)
    {
        const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        INFOMAN_NOHR(gfx);
        D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->ClearRenderTargetView(m_descHandle, clear_color_with_alpha, 0, nullptr));
    }
}