#include "IndexBuffer.h"

namespace Renderer
{
    IndexBuffer::IndexBuffer(D3D12RHI& gfx, std::vector<USHORT> pData)
        :
        IndexBuffer(gfx, "?", pData)
    {
    }

    IndexBuffer::IndexBuffer(D3D12RHI& gfx, std::string tag, std::vector<USHORT> pData)
        :
        m_tag(tag),
        m_numOfIndices(pData.size()),
        m_indexBufferSize(pData.size() * sizeof(pData[0]))
    {
        INFOMAN(gfx);

        // create committed resource (Index Buffer) for GPU access of Index data.
        {
            const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
            const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_indexBufferSize);
            D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr, IID_PPV_ARGS(&m_indexBuffer)
            ));
        }

        // create committed resource (Upload Buffer) for CPU upload of Index data.        
        {
            auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
            auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(m_indexBufferSize) };

            D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_indexUploadBuffer)
            ));
        }

        D3D12_SUBRESOURCE_DATA indexData = {};
        indexData.pData = reinterpret_cast<BYTE*>(pData.data());
        indexData.RowPitch = m_indexBufferSize;
        indexData.SlicePitch = indexData.RowPitch;

        UpdateSubresources(GetCommandList(gfx), m_indexBuffer.Get(), m_indexUploadBuffer.Get(), 0, 0, 1, &indexData);
        gfx.TransitionResource(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
        CreateView(gfx);
    }

    void IndexBuffer::CreateView(D3D12RHI& gfx)
    {
        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.SizeInBytes = m_indexBufferSize;
        m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    }

    void IndexBuffer::Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG)
    {
        INFOMAN_NOHR(gfx);
        D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->IASetIndexBuffer(&m_indexBufferView));
    }

    std::shared_ptr<IndexBuffer> IndexBuffer::Resolve(D3D12RHI& gfx, std::string tag, std::vector<USHORT> pData)
    {
        return Codex::Resolve<IndexBuffer>(gfx, tag, pData);
    }

    std::string IndexBuffer::GetUID() const noexcept(!IS_DEBUG)
    {
        return GenerateUID_(m_tag);
    }

    std::string IndexBuffer::GenerateUID_(const std::string& tag)
    {
        using namespace std::string_literals;
        return typeid(IndexBuffer).name() + "#"s + tag;
    }
}