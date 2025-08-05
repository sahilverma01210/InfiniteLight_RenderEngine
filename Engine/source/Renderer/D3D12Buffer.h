#pragma once
#include "GraphicsResource.h"

namespace Renderer
{
	enum class BufferType
	{
		Common,
		Vertex,
		Index,
		Constant,
		CPU_Only,
		GPU_Only,
		Raw
	};

	enum ResourceFlags : Uint32
	{
		None = 0,
		UnorderedAccess = 1 << 0,
		RT_AS = 1 << 1,
	};

	class D3D12Buffer : public D3D12Resource, public GraphicsResource
	{
	public:
		D3D12Buffer(D3D12RHI& gfx, const void* pData = nullptr, std::uint64_t bufferSize = 0, std::uint64_t strideSize = 0, Uint32 flags = 0, BufferType type = BufferType::Common);
		~D3D12Buffer();
		void UploadCPU(const void* pData, Uint64 bufferSize, Uint64 offset = 0) noexcept(!IS_DEBUG);
		void UpdateGPU(const void* pData, Uint64 bufferSize, Uint64 offset = 0) noexcept(!IS_DEBUG);
		BufferType GetType() const { return m_type; }
		void* GetCPUAddress() const { return m_mappedData; }

	public:
		D3D12RHI& m_gfx;
		BufferType m_type;
		void* m_mappedData = nullptr; // cpu adress of the upload buffer in CPU and GPU accesible memory (Usually RAM)
	};
}