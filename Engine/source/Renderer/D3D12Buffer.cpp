#include "D3D12Buffer.h"

namespace Renderer
{
    D3D12Buffer::D3D12Buffer(D3D12RHI& gfx, const void* pData, std::uint64_t bufferSize, std::uint64_t strideSize, Uint32 flags, BufferType type)
        :
		m_gfx(gfx),
        m_type(type)
	{
        INFOMAN(gfx);

        m_resourceType = (flags & ResourceFlags::RT_AS) ? ResourceType::AccelerationStructure : ResourceType::Buffer;
		m_viewType = D3D12Resource::ViewType::CBV;

        m_bufferSRV.FirstElement = 0;
        m_bufferSRV.Flags = type == BufferType::Raw ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
        m_bufferSRV.StructureByteStride = strideSize;
        strideSize = type == BufferType::Raw ? sizeof(Uint32) : strideSize;
        m_bufferSRV.NumElements = strideSize ? (std::uint32_t)bufferSize / strideSize : 0; // stride = Size of Struct

        // create committed resource (Upload Buffer) for CPU upload of buffer data.
        if (type != BufferType::GPU_Only)
        {
            auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize, (flags & ResourceFlags::UnorderedAccess) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE) };

            ALLOCATION_DESC allocation_desc{};
            allocation_desc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

            GetAllocator(gfx)->CreateResource(
                &allocation_desc,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                &m_cpuAllocation,
                IID_PPV_ARGS(&m_uploadBuffer)
            );

            D3D12_RANGE readRange{};        // We do not intend to read from this resource on the CPU, hence it is initialized with Begin = End = 0.
            D3D12RHI_THROW_INFO(m_uploadBuffer->Map(0, &readRange, &m_mappedData));
        }

        // create committed resource (Default Buffer) for GPU access of buffer data.
		if (type != BufferType::CPU_Only)
        {
            auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize, (flags & ResourceFlags::UnorderedAccess) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE) };

            ALLOCATION_DESC allocation_desc{};
            allocation_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

            D3D12_RESOURCE_STATES resource_state = D3D12_RESOURCE_STATE_COMMON;

            if (flags & ResourceFlags::RT_AS) resource_state = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

            GetAllocator(gfx)->CreateResource(
                &allocation_desc,
                &resourceDesc,
                resource_state,
                nullptr,
                &m_gpuAllocation,
                IID_PPV_ARGS(&m_resourceBuffer)
            );

            if (pData && type != BufferType::GPU_Only) UpdateGPU(pData, bufferSize, 0);
        }
        else
        {
            if (pData) UploadCPU(pData, bufferSize, 0);
        }
	}

    D3D12Buffer::~D3D12Buffer()
    {
		m_mappedData = nullptr;
    }

    void D3D12Buffer::UploadCPU(const void* pData, Uint64 bufferSize, Uint64 offset) noexcept(!IS_DEBUG)
    {
        assert(pData != nullptr && "D3D12Buffer::UploadCPU: pData must not be null!");

        INFOMAN(m_gfx);

        D3D12RHI_THROW_INFO_ONLY(memcpy((Uint8*)m_mappedData + offset, pData, bufferSize)); // m_mappedData is typecasted to Uint8* for correct pointer arithmetic (Move pointer by one Byte per offset).
    }

    void D3D12Buffer::UpdateGPU(const void* pData, Uint64 bufferSize, Uint64 offset) noexcept(!IS_DEBUG)
    {
        INFOMAN(m_gfx);

        // Upload Data to CPU Upload Memory
        if (pData) UploadCPU(pData, bufferSize, offset);

        // Copy Data to GPU Memory
        D3D12RHI_THROW_INFO_ONLY(GetCommandList(m_gfx)->CopyBufferRegion(m_resourceBuffer.Get(), offset, m_uploadBuffer.Get(), offset, bufferSize));
    }
}