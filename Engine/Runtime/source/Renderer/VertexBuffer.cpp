#include "VertexBuffer.h"

namespace Renderer
{
	VertexBuffer::VertexBuffer(D3D12RHI& gfx, UINT dataSize, const void* pData)
	{
        m_vertexBufferSize = dataSize;

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
        ComPtr<ID3D12Resource> vertexUploadBuffer;
        {
            auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
            auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize) };

            GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&vertexUploadBuffer));
        }

        // Copy the triangle data to the vertex buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        vertexUploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
        memcpy(pVertexDataBegin, pData, m_vertexBufferSize);
        vertexUploadBuffer->Unmap(0, nullptr);

        // reset command list and allocator  
        GetCommandAllocator(gfx)->Reset();
        GetCommandList(gfx)->Reset(GetCommandAllocator(gfx), nullptr);

        // copy Upload Buffer to Vertex Buffer 
        GetCommandList(gfx)->CopyResource(m_vertexBuffer.Get(), vertexUploadBuffer.Get());

        // transition vertex buffer to vertex buffer state 
        auto resourceBarrier0 = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        GetCommandList(gfx)->ResourceBarrier(1, &resourceBarrier0);

        // close command list and submit command list to queue as array with single element.
        GetCommandList(gfx)->Close();
        ID3D12CommandList* const commandLists[] = { GetCommandList(gfx) };
        GetCommandQueue(gfx)->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);

        InsertFence(gfx);

        CreateView(gfx);
	}

	void VertexBuffer::CreateView(D3D12RHI& gfx)
	{
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = m_vertexBufferSize;
	}

    void VertexBuffer::Bind(D3D12RHI& gfx) noexcept
    {
        GetCommandList(gfx)->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    }
}