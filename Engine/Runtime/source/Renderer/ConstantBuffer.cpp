#include "ConstantBuffer.h"

namespace Renderer
{
	ConstantBuffer::ConstantBuffer(D3D12RHI& gfx, UINT dataSize, const void* pData)
	{
        m_constantBufferSize = dataSize;

        // create committed resource (Index Buffer) for GPU access of Index data.
        {
            const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
            const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_constantBufferSize);
            GetDevice(gfx)->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr, IID_PPV_ARGS(&m_constantBuffer)
            );
        }

        // create committed resource (Upload Buffer) for CPU upload of Index data.
        ComPtr<ID3D12Resource> constantUploadBuffer;
        {
            auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
            auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(m_constantBufferSize) };

            GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&constantUploadBuffer));
        }

        // Copy the index data to the index buffer.
        UINT8* pConstantDataBegin;
        CD3DX12_RANGE readRangeC(0, 0);        // We do not intend to read from this resource on the CPU.
        constantUploadBuffer->Map(0, &readRangeC, reinterpret_cast<void**>(&pConstantDataBegin));
        memcpy(pConstantDataBegin, pData, m_constantBufferSize);
        constantUploadBuffer->Unmap(0, nullptr);

        // reset command list and allocator   
        GetCommandAllocator(gfx)->Reset();
        GetCommandList(gfx)->Reset(GetCommandAllocator(gfx), nullptr);

        // copy Upload Buffer to Index Buffer 
        GetCommandList(gfx)->CopyResource(m_constantBuffer.Get(), constantUploadBuffer.Get());

        // transition vertex buffer to vertex buffer state 
        auto resourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_constantBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        GetCommandList(gfx)->ResourceBarrier(1, &resourceBarrier2);

        // close command list and submit command list to queue as array with single element.
        GetCommandList(gfx)->Close();
        ID3D12CommandList* const commandLists[] = { GetCommandList(gfx) };
        GetCommandQueue(gfx)->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);

        InsertFence(gfx);
	}

	void ConstantBuffer::Bind(D3D12RHI& gfx) noexcept
	{
        GetCommandList(gfx)->SetGraphicsRootConstantBufferView(2, m_constantBuffer->GetGPUVirtualAddress());
	}
}