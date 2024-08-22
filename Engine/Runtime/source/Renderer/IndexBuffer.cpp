#include "IndexBuffer.h"

namespace Renderer
{
	IndexBuffer::IndexBuffer(D3D12RHI& gfx, UINT dataSize, const void* pData)
	{
        m_indexBufferSize = dataSize;

        // create committed resource (Index Buffer) for GPU access of Index data.
        {
            const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
            const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_indexBufferSize);
            GetDevice(gfx)->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr, IID_PPV_ARGS(&m_indexBuffer)
            );
        }

        // create committed resource (Upload Buffer) for CPU upload of Index data.
        ComPtr<ID3D12Resource> indexUploadBuffer;
        {
            auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
            auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(m_indexBufferSize) };

            GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&indexUploadBuffer));
        }

        // Copy the index data to the index buffer.
        UINT8* pIndexDataBegin;
        CD3DX12_RANGE readRangeI(0, 0);        // We do not intend to read from this resource on the CPU.
        indexUploadBuffer->Map(0, &readRangeI, reinterpret_cast<void**>(&pIndexDataBegin));
        memcpy(pIndexDataBegin, pData, m_indexBufferSize);
        indexUploadBuffer->Unmap(0, nullptr);

        // reset command list and allocator   
        GetCommandAllocator(gfx)->Reset();
        GetCommandList(gfx)->Reset(GetCommandAllocator(gfx), nullptr);

        // copy Upload Buffer to Index Buffer 
        GetCommandList(gfx)->CopyResource(m_indexBuffer.Get(), indexUploadBuffer.Get());

        // transition index buffer to index buffer state 
        auto resourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
        GetCommandList(gfx)->ResourceBarrier(1, &resourceBarrier1);

        // close command list and submit command list to queue as array with single element.
        GetCommandList(gfx)->Close();
        ID3D12CommandList* const commandLists[] = { GetCommandList(gfx) };
        GetCommandQueue(gfx)->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);

        InsertFence(gfx);

        CreateView(gfx);
	}

	void IndexBuffer::CreateView(D3D12RHI& gfx)
	{
        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.SizeInBytes = m_indexBufferSize;
        m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	}

	void IndexBuffer::Bind(D3D12RHI& gfx) noexcept
	{
        GetCommandList(gfx)->IASetIndexBuffer(&m_indexBufferView);
	}
}