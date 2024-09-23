#include "TextureBuffer.h"

namespace Renderer
{
	TextureBuffer::TextureBuffer(D3D12RHI& gfx, UINT rootParameterIndex, const WCHAR* filename, ID3D12DescriptorHeap* srvHeap, UINT offset)
        : 
        m_rootParameterIndex(rootParameterIndex),
        m_srvHeap(srvHeap),
        m_offset(offset)
	{
        // load image data from disk 
        ScratchImage image;
        HRESULT hr = LoadFromWICFile(filename, WIC_FLAGS_NONE, nullptr, image);

        /*if (FAILED(hr))
        {
            OutputDebugString(filename);
            OutputDebugString(std::to_wstring(hr).c_str());
        }*/

        // generate mip chain 
        ScratchImage mipChain;
        GenerateMipMaps(*image.GetImages(), TEX_FILTER_BOX, 0, mipChain);

        // collect subresource data
        std::vector<D3D12_SUBRESOURCE_DATA> subresourceData;
        {
            subresourceData.reserve(mipChain.GetImageCount());

            for (int i = 0; i < mipChain.GetImageCount(); ++i) {
                const auto img = mipChain.GetImage(i, 0, 0);
                subresourceData.push_back(D3D12_SUBRESOURCE_DATA{
                    .pData = img->pixels,
                    .RowPitch = (LONG_PTR)img->rowPitch,
                    .SlicePitch = (LONG_PTR)img->slicePitch,
                    });
            }
        }

        // create committed resource (Texture Buffer) for GPU access of Texture data.
        {
            auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
            const auto& chainBase = *mipChain.GetImages();
            auto resourceDesc = CD3DX12_RESOURCE_DESC{};
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            resourceDesc.Width = (UINT)chainBase.width;
            resourceDesc.Height = (UINT)chainBase.height;
            resourceDesc.DepthOrArraySize = 1;
            resourceDesc.MipLevels = (UINT16)mipChain.GetImageCount();
            resourceDesc.Format = chainBase.format;
            resourceDesc.SampleDesc = { .Count = 1 };
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                IID_PPV_ARGS(&m_texureBuffer)
            );
        }

        // create committed resource (Upload Buffer) for CPU upload of Index data.
        ComPtr<ID3D12Resource> texureUploadBuffer;
        {
            UINT texureUploadBufferSize = GetRequiredIntermediateSize(m_texureBuffer.Get(), 0, (UINT)subresourceData.size());
            auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
            auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(texureUploadBufferSize) };

            GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&texureUploadBuffer)
            );
        }

        // reset command list and allocator   
        GetCommandAllocator(gfx)->Reset();
        GetCommandList(gfx)->Reset(GetCommandAllocator(gfx), nullptr);

        // write commands to copy data to upload texture (copying each subresource). Copy the texture data to the texture buffer.
        UpdateSubresources(
            GetCommandList(gfx),
            m_texureBuffer.Get(),
            texureUploadBuffer.Get(),
            0, 0,
            (UINT)subresourceData.size(),
            subresourceData.data()
        );

        // Transition texture buffer to texture buffer state  
        auto resourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_texureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        GetCommandList(gfx)->ResourceBarrier(1, &resourceBarrier2);

        // close command list and submit command list to queue as array with single element.
        GetCommandList(gfx)->Close();
        ID3D12CommandList* const commandLists[] = { GetCommandList(gfx) };
        GetCommandQueue(gfx)->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);

        InsertFence(gfx);

        CreateView(gfx);
	}

    void TextureBuffer::CreateView(D3D12RHI& gfx)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = m_texureBuffer->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = m_texureBuffer->GetDesc().MipLevels;

        D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
        CPUHandle.ptr += GetDevice(gfx)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * m_offset;
        GetDevice(gfx)->CreateShaderResourceView(m_texureBuffer.Get(), &srvDesc, CPUHandle);
    }

    void TextureBuffer::Update(D3D12RHI& gfx, const void* pData) noexcept
    {
    }

	void TextureBuffer::Bind(D3D12RHI& gfx) noexcept
	{
        // bind the descriptor table containing the texture descriptor 
        GetCommandList(gfx)->SetGraphicsRootDescriptorTable(m_rootParameterIndex, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
	}

    bool TextureBuffer::HasAlpha() const noexcept
    {
        return hasAlpha;
    }
}