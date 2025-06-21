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
	};

	class D3D12Buffer : public D3D12Resource, public GraphicsResource
	{
	public:
		D3D12Buffer(D3D12RHI& gfx, const void* pData, std::uint64_t bufferSize, std::uint64_t strideSize = 0);
		~D3D12Buffer();
		void Update(D3D12RHI& gfx, const void* pData, std::uint64_t bufferSize, BufferType type = BufferType::Common) noexcept(!IS_DEBUG);

	private:
		ComPtr<ID3D12Resource> m_constantUploadBuffer;
	};
}