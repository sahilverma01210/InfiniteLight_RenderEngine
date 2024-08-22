#pragma once

#include "Bindable.h"

namespace Renderer 
{
	class ConstantBuffer : public Bindable
	{
	public:
		ConstantBuffer(D3D12RHI& gfx, UINT dataSize, const void* pData);
		void Bind(D3D12RHI& gfx) noexcept override;
	private:
		UINT m_constantBufferSize;
		ComPtr<ID3D12Resource> m_constantBuffer;
	};
}