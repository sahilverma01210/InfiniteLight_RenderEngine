#pragma once
#include "BindableCodex.h"

namespace Renderer
{
	class ShaderResourceView : public Bindable
	{
	public:
		ShaderResourceView(D3D12RHI& gfx, UINT rootParameterIndex, UINT numSRVDescriptors, UINT backwardRTOffset = 0);
		~ShaderResourceView() = default;
		ID3D12DescriptorHeap* GetSRVHeap();
		void AddTextureResource(D3D12RHI& gfx, UINT offset, ID3D12Resource* textureBuffer, bool isCubeMap = false);
		ID3D12Resource* GetResource(D3D12RHI& gfx);
		void Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;

	protected:
		UINT m_backwardRTOffset;
		UINT m_textureCount = 0;
		ID3D12Resource* m_buffer;
		UINT m_rootParameterIndex;
		ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	};
}