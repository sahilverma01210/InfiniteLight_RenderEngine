#include "ConstantBuffer.h"

namespace Renderer
{
    ConstantBuffer::ConstantBuffer(D3D12RHI& gfx, UINT rootParameterIndex, UINT dataSize, const void* pData) :
        m_rootParameterIndex(rootParameterIndex),
        m_constantBufferSize(dataSize)
	{
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
        {
            auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
            auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(m_constantBufferSize) };

            GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_constantUploadBuffer));
        }

        // Copy the index data to the index buffer.
        UINT8* pConstantDataBegin;
        CD3DX12_RANGE readRangeC(0, 0);        // We do not intend to read from this resource on the CPU.
        m_constantUploadBuffer->Map(0, &readRangeC, reinterpret_cast<void**>(&pConstantDataBegin));
        memcpy(pConstantDataBegin, pData, m_constantBufferSize);
        m_constantUploadBuffer->Unmap(0, nullptr);

        // reset command list and allocator   
        GetCommandAllocator(gfx)->Reset();
        GetCommandList(gfx)->Reset(GetCommandAllocator(gfx), nullptr);

        // copy Upload Buffer to Index Buffer 
        GetCommandList(gfx)->CopyResource(m_constantBuffer.Get(), m_constantUploadBuffer.Get());

        // transition vertex buffer to vertex buffer state 
        auto resourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_constantBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        GetCommandList(gfx)->ResourceBarrier(1, &resourceBarrier2);

        // close command list and submit command list to queue as array with single element.
        GetCommandList(gfx)->Close();
        ID3D12CommandList* const commandLists[] = { GetCommandList(gfx) };
        GetCommandQueue(gfx)->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);

        InsertFence(gfx);
	}

    ConstantBuffer::ConstantBuffer(D3D12RHI& gfx, UINT rootParameterIndex, Buffer dataBuffer)
        :
        ConstantBuffer(gfx, rootParameterIndex, (UINT)dataBuffer.GetRootLayoutElement().GetSizeInBytes(), (&dataBuffer)->GetData())
    {
        m_dataBuffer.emplace(dataBuffer);
    }

    void ConstantBuffer::Update(D3D12RHI& gfx, const void* pData) noexcept
    {
        // Copy the index data to the index buffer.
        UINT8* pConstantDataBegin;
        CD3DX12_RANGE readRangeC(0, 0);        // We do not intend to read from this resource on the CPU.
        m_constantUploadBuffer->Map(0, &readRangeC, reinterpret_cast<void**>(&pConstantDataBegin));
        memcpy(pConstantDataBegin, pData, m_constantBufferSize);
        m_constantUploadBuffer->Unmap(0, nullptr);

        // transition vertex buffer to vertex buffer state 
        auto resourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(m_constantBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
        GetCommandList(gfx)->ResourceBarrier(1, &resourceBarrier1);

        // copy Upload Buffer to Index Buffer 
        GetCommandList(gfx)->CopyResource(m_constantBuffer.Get(), m_constantUploadBuffer.Get());

        // transition vertex buffer to vertex buffer state 
        auto resourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_constantBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        GetCommandList(gfx)->ResourceBarrier(1, &resourceBarrier2);

        InsertFence(gfx);
    }

    void ConstantBuffer::Update(D3D12RHI& gfx, Buffer dataBuffer) noexcept
    {
        Update(gfx, dataBuffer.GetData());
    }

	void ConstantBuffer::Bind(D3D12RHI& gfx) noexcept
	{
        if (dirty)
        {
            Update(gfx, m_dataBuffer.value());
            dirty = false;
        }

        GetCommandList(gfx)->SetGraphicsRootConstantBufferView(m_rootParameterIndex, m_constantBuffer->GetGPUVirtualAddress());
	}

    Buffer ConstantBuffer::GetBuffer() const noexcept
    {
        return m_dataBuffer.value();
    }

    void ConstantBuffer::SetBuffer(const Buffer& buf_in)
    {
        m_dataBuffer.value().CopyFrom(buf_in);
        dirty = true;
    }

    void ConstantBuffer::Accept(TechniqueProbe& probe)
    {
        if (m_dataBuffer.has_value())
        {
            if (probe.VisitBuffer(m_dataBuffer.value()))
            {
                dirty = true;
            }
        }
    }
}