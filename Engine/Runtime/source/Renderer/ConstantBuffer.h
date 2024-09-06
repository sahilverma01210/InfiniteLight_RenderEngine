#pragma once

#include "Bindable.h"

namespace Renderer 
{
	class ConstantBuffer : public Bindable
	{
	public:
		ConstantBuffer(D3D12RHI& gfx, UINT rootParameterIndex, UINT dataSize, const void* pData);
		void Update(D3D12RHI& gfx, const void* pData) noexcept override;
		void Bind(D3D12RHI& gfx) noexcept override;
	private:
		UINT m_rootParameterIndex;
		UINT m_constantBufferSize;
		ComPtr<ID3D12Resource> m_constantBuffer;
		ComPtr<ID3D12Resource> m_constantUploadBuffer;
	};
}