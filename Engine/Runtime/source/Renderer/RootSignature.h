#pragma once
#include "Bindable.h"

namespace Renderer
{
	class RootSignature : public Bindable
	{
	public:
		RootSignature(D3D12RHI& gfx, UINT numConstants, UINT numConstantBufferViews, UINT numShaderResourceViews);
		ID3D12RootSignature* GetRootSignature();
		void Bind(D3D12RHI& gfx) noexcept override;
	private:
		ComPtr<ID3D12RootSignature> m_rootSignature;
	};
}