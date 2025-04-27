#include "TextureBuffer.h"

namespace Renderer
{
    MeshTextureBuffer::MeshTextureBuffer(D3D12RHI& gfx, std::string filename)
        :
        m_filename(filename)
    {
        INFOMAN(gfx);

        if (filename != "NULL_TEX")
        {
            // load image data from disk 
            ScratchImage image;
            D3D12RHI_THROW_INFO(LoadFromWICFile(std::wstring(filename.begin(), filename.end()).c_str(), WIC_FLAGS_IGNORE_SRGB, nullptr, image));

            D3D12RHI_THROW_INFO(GenerateMipMaps(*image.GetImages(), TEX_FILTER_BOX, 0, m_mipChain));
            m_hasAlpha = !m_mipChain.IsAlphaAllOpaque();
        }
        else
        {
            m_mipChain.Initialize2D(m_format = DXGI_FORMAT_R8G8B8A8_UNORM, gfx.GetWidth(), gfx.GetHeight(), 1, 1);
        }

        // collect subresource data
        std::vector<D3D12_SUBRESOURCE_DATA> subresourceData;
        {
            subresourceData.reserve(m_mipChain.GetImageCount());

            for (int i = 0; i < m_mipChain.GetImageCount(); ++i) {
                const auto img = m_mipChain.GetImage(i, 0, 0);
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
            const auto& chainBase = *m_mipChain.GetImages();
            auto resourceDesc = CD3DX12_RESOURCE_DESC{};
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            resourceDesc.Width = (UINT)chainBase.width;
            resourceDesc.Height = (UINT)chainBase.height;
            resourceDesc.DepthOrArraySize = 1;
            resourceDesc.MipLevels = (UINT16)m_mipChain.GetImageCount();
            resourceDesc.Format = chainBase.format;
            resourceDesc.SampleDesc = { .Count = 1 };
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

            D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                IID_PPV_ARGS(&m_resourceBuffer)
            ));
        }

        // create committed resource (Upload Buffer) for CPU upload of Index data.        
        {
            UINT texureUploadBufferSize = GetRequiredIntermediateSize(m_resourceBuffer.Get(), 0, (UINT)subresourceData.size());
            auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
            auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(texureUploadBufferSize) };

            D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_texureUploadBuffer)
            ));
        }

        // write commands to copy data to upload texture (copying each subresource). Copy the texture data to the texture buffer.
        UpdateSubresources(
            GetCommandList(gfx),
            m_resourceBuffer.Get(),
            m_texureUploadBuffer.Get(),
            0, 0,
            (UINT)subresourceData.size(),
            subresourceData.data()
        );

        gfx.TransitionResource(m_resourceBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    bool MeshTextureBuffer::HasAlpha() const noexcept(!IS_DEBUG)
    {
        return m_hasAlpha;
    }

    std::shared_ptr<MeshTextureBuffer> MeshTextureBuffer::Resolve(D3D12RHI& gfx, std::string filename)
    {
        return Codex::Resolve<MeshTextureBuffer>(gfx, filename);
    }

    std::string MeshTextureBuffer::GenerateUID(std::string filename)
    {
        using namespace std::string_literals;
        return typeid(MeshTextureBuffer).name() + "#"s + filename;
    }

    std::string MeshTextureBuffer::GetUID() const noexcept(!IS_DEBUG)
    {
        return GenerateUID(m_filename);
    }

    CubeMapTextureBuffer::CubeMapTextureBuffer(D3D12RHI& gfx, const WCHAR* foldername)
        :
        m_foldername(foldername)
    {
        INFOMAN(gfx);

        // load image data from disk 
        ScratchImage images[6];
        for (int i = 0; i < 6; i++)
        {
            std::wstring path = std::wstring(foldername) + L"\\" + std::to_wstring(i) + L".png";
            D3D12RHI_THROW_INFO(LoadFromWICFile(path.c_str(), WIC_FLAGS_IGNORE_SRGB, nullptr, images[i]));
        }        

        // collect subresource data
        std::vector<D3D12_SUBRESOURCE_DATA> subresourceData;
        {
            for (int i = 0; i < 6; ++i) {
                const auto img = images[i].GetImage(0, 0, 0);
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
            const auto& image = *images[0].GetImages();
            auto resourceDesc = CD3DX12_RESOURCE_DESC{};
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            resourceDesc.Width = (UINT)image.width;
            resourceDesc.Height = (UINT)image.height;
            resourceDesc.DepthOrArraySize = (UINT)subresourceData.size();
            resourceDesc.MipLevels = 1;
            resourceDesc.Format = image.format;
            resourceDesc.SampleDesc = { .Count = 1 };
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                IID_PPV_ARGS(&m_resourceBuffer)
            ));
        }

        // create committed resource (Upload Buffer) for CPU upload of Index data.
        {
            UINT texureUploadBufferSize = GetRequiredIntermediateSize(m_resourceBuffer.Get(), 0, (UINT)subresourceData.size());
            auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
            auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(texureUploadBufferSize) };

            D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_texureUploadBuffer)
            ));
        }

        // write commands to copy data to upload texture (copying each subresource). Copy the texture data to the texture buffer.
        UpdateSubresources(
            GetCommandList(gfx),
            m_resourceBuffer.Get(),
            m_texureUploadBuffer.Get(),
            0, 0,
            (UINT)subresourceData.size(),
            subresourceData.data()
        );

        gfx.TransitionResource(m_resourceBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    std::shared_ptr<CubeMapTextureBuffer> CubeMapTextureBuffer::Resolve(D3D12RHI& gfx, const WCHAR* filename)
    {
        return Codex::Resolve<CubeMapTextureBuffer>(gfx, filename);
    }

    std::string CubeMapTextureBuffer::GenerateUID(const WCHAR* filename)
    {
        std::wstring wstringFileName = std::wstring(filename);
        std::string stringFileName = std::string(wstringFileName.begin(), wstringFileName.end());

        using namespace std::string_literals;
        return typeid(CubeMapTextureBuffer).name() + "#"s + stringFileName;
    }

    std::string CubeMapTextureBuffer::GetUID() const noexcept(!IS_DEBUG)
    {
        return GenerateUID(m_foldername);
    }

    DepthCubeMapTextureBuffer::DepthCubeMapTextureBuffer(D3D12RHI& gfx, UINT size)
    {
        INFOMAN(gfx);

        {
            auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
            auto resourceDesc = CD3DX12_RESOURCE_DESC{};
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            resourceDesc.Width = (UINT)size;
            resourceDesc.Height = (UINT)size;
            resourceDesc.DepthOrArraySize = 6;
            resourceDesc.MipLevels = 1;
            resourceDesc.Format = m_format = DXGI_FORMAT_R32_TYPELESS;
            resourceDesc.SampleDesc = { .Count = 1 };
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

            D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                nullptr,
                IID_PPV_ARGS(&m_resourceBuffer)
            ));
        }

        // make depth buffer resources for capturing shadow map
        for (UINT face = 0; face < 6; face++)
        {
            m_depthBuffers.push_back(std::make_shared<DepthStencil>(gfx, m_resourceBuffer.Get(), face));
        }
    }

    std::shared_ptr<DepthStencil> DepthCubeMapTextureBuffer::GetDepthBuffer(size_t index) const
    {
        return m_depthBuffers[index];
    }
}