#pragma once

#include "Bindable.h"

namespace Renderer
{
	class VertexBuffer : public Bindable
	{
	public:
		VertexBuffer(D3D12RHI& gfx, UINT dataSize, const void* pData);
		void CreateView(D3D12RHI& gfx);
		void Bind(D3D12RHI& gfx) noexcept override;
	private:
		UINT m_vertexBufferSize;
		ComPtr<ID3D12Resource> m_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	};
}