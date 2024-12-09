#pragma once
#include "BindableCodex.h"

namespace Renderer
{
	class RootSignature : public Bindable
	{
	public:
		RootSignature(D3D12RHI& gfx, PipelineDescription& pipelineDesc);
		ID3D12RootSignature* GetRootSignature();
		void Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;

	private:
		UINT m_numRootParameters;
		CD3DX12_ROOT_PARAMETER* m_rootParameters;
		ComPtr<ID3D12RootSignature> m_rootSignature;
	};
}