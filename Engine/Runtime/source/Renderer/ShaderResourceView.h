#pragma once
#include "Bindable.h"

namespace Renderer
{
	class ShaderResourceView : public Bindable
	{
	public:
		ShaderResourceView(D3D12RHI& gfx, UINT offset, UINT rootParameterIndex, ID3D12Resource* texureBuffer, ID3D12DescriptorHeap* srvHeap);
		void Update(D3D12RHI& gfx, const void* pData) noexcept override;
		void Bind(D3D12RHI& gfx) noexcept override;
	protected:
		UINT m_offset;
		UINT m_rootParameterIndex;
		ID3D12DescriptorHeap* m_srvHeap;
		ID3D12Resource* m_texureBuffer;
	};
}