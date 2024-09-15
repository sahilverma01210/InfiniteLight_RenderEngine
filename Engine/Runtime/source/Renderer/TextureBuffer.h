#pragma once

#include "Bindable.h"
#include "ShaderResourceView.h"

namespace Renderer
{
	class TextureBuffer : public Bindable
	{
	public:
		TextureBuffer(D3D12RHI& gfx, UINT rootParameterIndex, const WCHAR* filename, ID3D12DescriptorHeap* srvHeap, UINT offset);
		void Update(D3D12RHI& gfx, const void* pData) noexcept override;
		void Bind(D3D12RHI& gfx) noexcept override;
	private:
		UINT m_rootParameterIndex;
		UINT m_offset;
		ComPtr<ID3D12Resource> m_texureBuffer;
		ID3D12DescriptorHeap* m_srvHeap;
		std::unique_ptr<ShaderResourceView> m_shaderResourceView;
	};
}