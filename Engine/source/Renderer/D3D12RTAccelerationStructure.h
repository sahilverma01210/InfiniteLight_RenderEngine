#pragma once
#include "GraphicsResource.h"
#include "D3D12Buffer.h"

namespace Renderer
{
	struct D3D12RTGeometry
	{
		D3D12Buffer* vertexBuffer;
		Uint32 vertexBufferOffset;
		Uint32 vertexCount;
		Uint32 vertexStride;
		DXGI_FORMAT vertexFormat;

		D3D12Buffer* indexBuffer;
		Uint32 indexBufferOffset;
		Uint32 indexCount;
		DXGI_FORMAT indexFormat;

		Bool opaque;
	};

	class D3D12RTBottomLevelAS
	{
	public:
		D3D12RTBottomLevelAS(D3D12RHI& gfx, std::span<D3D12RTGeometry> geometries);
		D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress() const { return m_resultBuffer->GetGPUAddress(); }
		std::shared_ptr<D3D12Buffer> GetBuffer() const { return m_resultBuffer; }

	private:
		std::shared_ptr<D3D12Buffer> m_resultBuffer;
		std::shared_ptr<D3D12Buffer> m_scratchBuffer;
	};

	struct D3D12RTInstance
	{
		D3D12RTBottomLevelAS* blas;
		Float transform[4][4];
		Uint32 instanceId;
		Uint8 instanceMask;
	};

	class D3D12RTTopLevelAS
	{
	public:
		D3D12RTTopLevelAS(D3D12RHI& gfx, std::span<D3D12RTInstance> instances);
		D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress() const { return m_resultBuffer->GetGPUAddress(); }
		std::shared_ptr<D3D12Buffer> GetBuffer() const { return m_resultBuffer; }

	private:
		std::shared_ptr<D3D12Buffer> m_resultBuffer;
		std::shared_ptr<D3D12Buffer> m_scratchBuffer;
		std::shared_ptr<D3D12Buffer> m_instanceBuffer;
	};
}