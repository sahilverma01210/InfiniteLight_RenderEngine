#pragma once
#include "Bindable.h"

namespace Renderer
{
	class ShaderResourceView : public Bindable
	{
	public:
		ShaderResourceView(D3D12RHI& gfx, UINT rootParameterIndex, UINT numSRVDescriptors);
		ID3D12DescriptorHeap* GetSRVHeap();
		void AddResource(D3D12RHI& gfx, UINT offset, ID3D12Resource* texureBuffer, bool isCubeMap = false);
		void AddBackBufferAsResource(D3D12RHI& gfx);
		void Bind(D3D12RHI& gfx) noexcept override;

	protected:
		UINT m_rootParameterIndex;
		ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	};
}