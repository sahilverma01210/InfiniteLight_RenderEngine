#pragma once

#include "Bindable.h"

namespace Renderer
{
	class IndexBuffer : public Bindable
	{
	public:
		IndexBuffer(D3D12RHI& gfx, UINT dataSize, std::vector<USHORT> pData);
		void CreateView(D3D12RHI& gfx);
		void Update(D3D12RHI& gfx, const void* pData) noexcept override;
		void Bind(D3D12RHI& gfx) noexcept override;
	private:
		UINT m_indexBufferSize;
		ComPtr<ID3D12Resource> m_indexBuffer;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	};
}