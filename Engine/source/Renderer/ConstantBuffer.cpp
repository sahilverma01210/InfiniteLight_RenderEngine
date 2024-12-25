#include "ConstantBuffer.h"

namespace Renderer
{
    ConstantBuffer::ConstantBuffer(D3D12RHI& gfx, UINT dataSize, const void* pData) :
        m_constantBufferSize(dataSize)
	{
        INFOMAN(gfx);

        // Upload Data to CPU Memory
        {
            // create committed resource (Upload Buffer) for CPU upload of buffer data.
            {
                auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
                auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(m_constantBufferSize) };

                D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateCommittedResource(
                    &heapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &resourceDesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&m_constantUploadBuffer)));
            }

            // Copy the data to buffer.
            UINT8* pConstantDataBegin;
            CD3DX12_RANGE readRangeC(0, 0);        // We do not intend to read from this resource on the CPU.
            D3D12RHI_THROW_INFO(m_constantUploadBuffer->Map(0, &readRangeC, reinterpret_cast<void**>(&pConstantDataBegin)));
            D3D12RHI_THROW_INFO_ONLY(memcpy(pConstantDataBegin, pData, m_constantBufferSize));
            D3D12RHI_THROW_INFO_ONLY(m_constantUploadBuffer->Unmap(0, nullptr));
        }

        // Copy Data to GPU Memory
        {
            // create committed resource for GPU access of buffer data.
            {
                const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
                const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_constantBufferSize);
                D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateCommittedResource(
                    &heapProps,
                    D3D12_HEAP_FLAG_NONE,
                    &resourceDesc,
                    D3D12_RESOURCE_STATE_COPY_DEST,
                    nullptr, IID_PPV_ARGS(&m_constantBuffer)
                ));
            }

            gfx.ResetCommandList();

            // copy Upload Buffer to Index Buffer 
            D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->CopyResource(m_constantBuffer.Get(), m_constantUploadBuffer.Get()));

            gfx.TransitionResource(m_constantBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
            gfx.ExecuteCommandList();

            gfx.InsertFence();
        }
	}

    void ConstantBuffer::Update(D3D12RHI& gfx, const void* pData) noexcept(!IS_DEBUG)
    {
        INFOMAN(gfx);

        // Upload Data to CPU Memory
        {
            UINT8* pConstantDataBegin;
            CD3DX12_RANGE readRangeC(0, 0);        // We do not intend to read from this resource on the CPU.
            D3D12RHI_THROW_INFO(m_constantUploadBuffer->Map(0, &readRangeC, reinterpret_cast<void**>(&pConstantDataBegin)));
            D3D12RHI_THROW_INFO_ONLY(memcpy(pConstantDataBegin, pData, m_constantBufferSize));
            D3D12RHI_THROW_INFO_ONLY(m_constantUploadBuffer->Unmap(0, nullptr));
        }

        // Copy Data to GPU Memory
        {
            gfx.TransitionResource(m_constantBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);

            D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->CopyResource(m_constantBuffer.Get(), m_constantUploadBuffer.Get()));

            gfx.TransitionResource(m_constantBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

            gfx.InsertFence();
        }
    }

	void ConstantBuffer::Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
        if (m_selected)
        {
            Update(gfx, m_dataBuffer.value().GetData());
            m_selected = false;
        }
	}

    ID3D12Resource* ConstantBuffer::GetBuffer() const noexcept(!IS_DEBUG)
    {
        return m_constantBuffer.Get();
    }

    Buffer ConstantBuffer::GetDynamicBuffer() const noexcept(!IS_DEBUG)
    {
        return m_dataBuffer.value();
    }

    void ConstantBuffer::SetBuffer(const Buffer& buf_in)
    {
        m_dataBuffer.value().CopyFrom(buf_in);
        m_selected = true;
    }

    void ConstantBuffer::Accept(TechniqueProbe& probe)
    {
        if (m_dataBuffer.has_value())
        {
            //if (probe.VisitBuffer(m_dataBuffer.value()))
            //{
            //    m_selected = true;
            //}
        }
    }
}