#pragma once

#include "Bindable.h"
#include "Vertex.h"

namespace Renderer
{
	template <typename T>
	class VertexBuffer : public Bindable
	{
	public:
		VertexBuffer(D3D12RHI& gfx, UINT dataSize, std::vector<T> pData);
		VertexBuffer(D3D12RHI& gfx, const TemplateMeta::VertexBuffer& vbuf);
		void CreateView(D3D12RHI& gfx, UINT strides);
		void Update(D3D12RHI& gfx, const void* pData) noexcept override;
		void Bind(D3D12RHI& gfx) noexcept override;
	private:
		UINT m_vertexBufferSize;
		ComPtr<ID3D12Resource> m_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	};
}