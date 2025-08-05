#include "D3D12Texture.h"

namespace Renderer
{
    MeshTexture::MeshTexture(D3D12RHI& gfx, std::string filename, TextureDesc desc)
        :
		m_gfx(gfx),
        m_filename(filename)
    {
        INFOMAN(m_gfx);

        m_resourceType = ResourceType::Texture2D;
		m_isSRGB = desc.isSRGB;

        if (filename != "NULL_TEX")
        {
            // load image data from disk 
            ScratchImage image;
            D3D12RHI_THROW_INFO(LoadFromWICFile(std::wstring(filename.begin(), filename.end()).c_str(), WIC_FLAGS_NONE, nullptr, image));

            D3D12RHI_THROW_INFO(GenerateMipMaps(*image.GetImages(), TEX_FILTER_BOX, 0, m_mipChain));

			m_viewType = D3D12Resource::ViewType::SRV;
        }
        else
        {
            m_mipChain.Initialize2D(m_format = desc.format, m_gfx.GetWidth(), m_gfx.GetHeight(), 1, 1);

            m_viewType = D3D12Resource::ViewType::UAV;
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

            D3D12RHI_THROW_INFO(GetDevice(m_gfx)->CreateCommittedResource(
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

            D3D12RHI_THROW_INFO(GetDevice(m_gfx)->CreateCommittedResource(
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
            GetCommandList(m_gfx),
            m_resourceBuffer.Get(),
            m_texureUploadBuffer.Get(),
            0, 0,
            (UINT)subresourceData.size(),
            subresourceData.data()
        );

        m_gfx.TransitionResource(m_resourceBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    CubeMapTexture::CubeMapTexture(D3D12RHI& gfx, const WCHAR* foldername)
        :
        m_gfx(gfx),
        m_foldername(foldername)
    {
        INFOMAN(m_gfx);

        m_resourceType = ResourceType::TextureCube;
        m_viewType = D3D12Resource::ViewType::SRV;		

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
            subresourceData.reserve(6);

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
            resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

            D3D12RHI_THROW_INFO(GetDevice(m_gfx)->CreateCommittedResource(
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

            D3D12RHI_THROW_INFO(GetDevice(m_gfx)->CreateCommittedResource(
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
            GetCommandList(m_gfx),
            m_resourceBuffer.Get(),
            m_texureUploadBuffer.Get(),
            0, 0,
            (UINT)subresourceData.size(),
            subresourceData.data()
        );

        m_gfx.TransitionResource(m_resourceBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
}