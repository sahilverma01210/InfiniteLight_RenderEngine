#pragma once

#include "Bindable.h"

namespace Renderer
{
	template <typename T>
	class VertexBuffer : public Bindable
	{
	public:
		VertexBuffer(D3D12RHI& gfx, UINT dataSize, std::vector<T> pData);
		void CreateView(D3D12RHI& gfx, UINT strides);
		void Bind(D3D12RHI& gfx) noexcept override;
	private:
		UINT m_vertexBufferSize;
		ComPtr<ID3D12Resource> m_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	};
}