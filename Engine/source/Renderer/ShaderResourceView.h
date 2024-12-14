#pragma once
#include "BindableCodex.h"

namespace Renderer
{
	class ShaderResourceView : public Bindable
	{
	public:
		ShaderResourceView(D3D12RHI& gfx, UINT rootParameterIndex, UINT numSRVDescriptors);
		~ShaderResourceView() = default;
		ID3D12DescriptorHeap* GetSRVHeap();
		void AddTextureResource(D3D12RHI& gfx, UINT offset, ID3D12Resource* texureBuffer, bool isCubeMap = false);
		void AddBackBufferAsResource(D3D12RHI& gfx);
		void Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;

	protected:
		UINT m_rootParameterIndex;
		ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	};
}