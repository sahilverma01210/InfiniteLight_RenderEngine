#include "VertexBuffer.h"

namespace Renderer
{
    VertexBuffer::VertexBuffer(D3D12RHI& gfx, const char* data, UINT size, UINT layoutSize)
        :
        VertexBuffer(gfx, "?", data, size, layoutSize)
    {
    }

    VertexBuffer::VertexBuffer(D3D12RHI& gfx, const std::string& tag, const char* data, UINT size, UINT layoutSize)
        :
        tag(tag),
        m_vertexBufferSize(size)
    {
        // create committed resource (Vertex Buffer) for GPU access of vertex data.
        {
            const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
            const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize);
            GetDevice(gfx)->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr, IID_PPV_ARGS(&m_vertexBuffer)
            );
        }

        // create committed resource (Upload Buffer) for CPU upload of vertex data.
        {
            auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
            auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize) };

            GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_vertexUploadBuffer));
        }

        D3D12_SUBRESOURCE_DATA vertexData = {};
        vertexData.pData = data;
        vertexData.RowPitch = m_vertexBufferSize;
        vertexData.SlicePitch = vertexData.RowPitch;

        gfx.ResetCommandList();

        UpdateSubresources(GetCommandList(gfx), m_vertexBuffer.Get(), m_vertexUploadBuffer.Get(), 0, 0, 1, &vertexData);

        gfx.TransitionResource(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        gfx.ExecuteCommandList();

        InsertFence(gfx);

        CreateView(gfx, layoutSize);
    }

    void VertexBuffer::CreateView(D3D12RHI& gfx, UINT strides)
    {
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = strides;
        m_vertexBufferView.SizeInBytes = m_vertexBufferSize;
    }

    void VertexBuffer::Update(D3D12RHI& gfx, const void* pData) noexcept
    {
        // Copy the data to the buffer.
        UINT8* pConstantDataBegin;
        CD3DX12_RANGE readRangeC(0, 0);        // We do not intend to read from this resource on the CPU.
        m_vertexUploadBuffer->Map(0, &readRangeC, reinterpret_cast<void**>(&pConstantDataBegin));
        memcpy(pConstantDataBegin, pData, m_vertexBufferSize);
        m_vertexUploadBuffer->Unmap(0, nullptr);

        gfx.TransitionResource(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);

        GetCommandList(gfx)->CopyResource(m_vertexBuffer.Get(), m_vertexUploadBuffer.Get());

        gfx.TransitionResource(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        InsertFence(gfx);
    }

    void VertexBuffer::Bind(D3D12RHI& gfx) noexcept
    {
        GetCommandList(gfx)->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    }

    std::shared_ptr<VertexBuffer> VertexBuffer::Resolve(D3D12RHI& gfx, const std::string& tag, const char* data, UINT size, UINT layoutSize)
    {
        return Codex::Resolve<VertexBuffer>(gfx, tag, data, size, layoutSize);
    }

    std::string VertexBuffer::GenerateUID_(const std::string& tag)
    {
        using namespace std::string_literals;
        return typeid(VertexBuffer).name() + "#"s + tag;
    }

    std::string VertexBuffer::GetUID() const noexcept
    {
        return GenerateUID(tag);
    }
}