#pragma once
#include "Bindable.h"

namespace Renderer
{
	class ShaderResourceView : public Bindable
	{
	public:
		ShaderResourceView(D3D12RHI& gfx, UINT rootParameterIndex, UINT numSRVDescriptors);
		ID3D12DescriptorHeap* GetSRVHeap();
		void AddResource(D3D12RHI& gfx, UINT offset, ID3D12Resource* texureBuffer);
		void Update(D3D12RHI& gfx, const void* pData) noexcept override;
		void Bind(D3D12RHI& gfx) noexcept override;
	protected:
		UINT m_rootParameterIndex;
		ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	};
}