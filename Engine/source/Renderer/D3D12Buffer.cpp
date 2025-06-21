#include "D3D12Buffer.h"

namespace Renderer
{
    D3D12Buffer::D3D12Buffer(D3D12RHI& gfx, const void* pData, std::uint64_t bufferSize, std::uint64_t strideSize)
	{
        INFOMAN(gfx);

        m_resourceType = ResourceType::Buffer;
		m_viewType = D3D12Resource::ViewType::CBV;

        m_bufferSRV.FirstElement = 0;
        m_bufferSRV.NumElements = strideSize ? (std::uint32_t)bufferSize / strideSize : 0; // stride = Size of Struct
        m_bufferSRV.StructureByteStride = strideSize;


        // create committed resource (Upload Buffer) for CPU upload of buffer data.
        {
            auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize) };

            ALLOCATION_DESC allocation_desc{};
            allocation_desc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

            Allocation* alloc = nullptr;
            GetAllocator(gfx)->CreateResource(
                &allocation_desc,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                &alloc,
                IID_PPV_ARGS(&m_constantUploadBuffer)
            );
        }

        // create committed resource (Default Buffer) for GPU access of buffer data.
        {
            auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize) };

            ALLOCATION_DESC allocation_desc{};
            allocation_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

            Allocation* alloc = nullptr;
            GetAllocator(gfx)->CreateResource(
                &allocation_desc,
                &resourceDesc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                &alloc,
                IID_PPV_ARGS(&m_resourceBuffer)
            );
        }

		Update(gfx, pData, bufferSize);
	}

    D3D12Buffer::~D3D12Buffer()
    {
    }

    void D3D12Buffer::Update(D3D12RHI& gfx, const void* pData, std::uint64_t bufferSize, BufferType type) noexcept(!IS_DEBUG)
    {
        INFOMAN(gfx);

        // Upload Data to CPU Memory
        {
            void* mappedData = nullptr;
            D3D12_RANGE readRange{};        // We do not intend to read from this resource on the CPU.
            D3D12RHI_THROW_INFO(m_constantUploadBuffer->Map(0, &readRange, &mappedData));
            D3D12RHI_THROW_INFO_ONLY(memcpy(mappedData, pData, bufferSize));
            D3D12RHI_THROW_INFO_ONLY(m_constantUploadBuffer->Unmap(0, nullptr));
        }

        // Copy Data to GPU Memory
        {
            D3D12_RESOURCE_STATES currentState = D3D12_RESOURCE_STATE_COMMON;

            switch (type)
            {
            case BufferType::Vertex:
				currentState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
                break;
            case BufferType::Index:
				currentState = D3D12_RESOURCE_STATE_INDEX_BUFFER;
                break;
            case BufferType::Constant:
                currentState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
                break;
            }

            gfx.TransitionResource(m_resourceBuffer.Get(), currentState, D3D12_RESOURCE_STATE_COPY_DEST);

            D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->CopyResource(m_resourceBuffer.Get(), m_constantUploadBuffer.Get()));

            gfx.TransitionResource(m_resourceBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, currentState);
        }
    }
}