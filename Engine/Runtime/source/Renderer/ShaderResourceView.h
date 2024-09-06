#pragma once
#include "Bindable.h"

namespace Renderer
{
	class ShaderResourceView : public Bindable
	{
	public:
		ShaderResourceView(D3D12RHI& gfx);
		ID3D12DescriptorHeap* GetShaderResourceView();
		void Update(D3D12RHI& gfx, const void* pData) noexcept override;
		void Bind(D3D12RHI& gfx) noexcept override;
	protected:
		ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	};
}