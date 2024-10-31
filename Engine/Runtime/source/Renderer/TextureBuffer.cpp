#include "TextureBuffer.h"
#include "BindableCodex.h"

namespace Renderer
{
    TextureBuffer::TextureBuffer(D3D12RHI& gfx, const WCHAR* filename)
        :
        m_filename(filename)
    {
        // load image data from disk 
        ScratchImage image;
        HRESULT hr = LoadFromWICFile(filename, WIC_FLAGS_IGNORE_SRGB, nullptr, image);

        /*if (FAILED(hr))
        {
            OutputDebugString(filename);
            OutputDebugString(std::to_wstring(hr).c_str());
        }*/

        // generate mip chain 
        ScratchImage mipChain;
        GenerateMipMaps(*image.GetImages(), TEX_FILTER_BOX, 0, mipChain);
        hasAlpha = !mipChain.IsAlphaAllOpaque();

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
    }

    void TextureBuffer::Update(D3D12RHI& gfx, const void* pData) noexcept
    {
    }

    void TextureBuffer::Bind(D3D12RHI& gfx) noexcept
    {
        
    }

    ID3D12Resource* TextureBuffer::GetBuffer()
    {
        return m_texureBuffer.Get();
    }

    bool TextureBuffer::HasAlpha() const noexcept
    {
        return hasAlpha;
    }

    bool TextureBuffer::HasAlphaChannel(const DirectX::Image& image) {
        switch (image.format) {
            // Common formats with alpha channel
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_A8_UNORM:
            return true;

            // Formats without alpha channel
        default:
            return false;
        }
    }

    std::shared_ptr<TextureBuffer> TextureBuffer::Resolve(D3D12RHI& gfx, const WCHAR* filename)
    {
        return Codex::Resolve<TextureBuffer>(gfx, filename);
    }

    std::string TextureBuffer::GenerateUID(const WCHAR* filename)
    {
        std::wstring wstringFileName = std::wstring(filename);
        std::string stringFileName = std::string(wstringFileName.begin(), wstringFileName.end());

        using namespace std::string_literals;
        return typeid(TextureBuffer).name() + "#"s + stringFileName;
    }

    std::string TextureBuffer::GetUID() const noexcept
    {
        return GenerateUID(m_filename);
    }
}