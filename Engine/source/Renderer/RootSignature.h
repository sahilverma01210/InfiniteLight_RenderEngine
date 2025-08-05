#pragma once
#include "GraphicsResource.h"

namespace Renderer
{
	class RootSignature : public GraphicsResource
	{
	public:
		RootSignature(D3D12RHI& gfx, PipelineDescription& pipelineDesc);
		~RootSignature() = default;
		ID3D12RootSignature* GetRootSignature();
		void Bind() noexcept(!IS_DEBUG);

	private:
		D3D12RHI& m_gfx;
		PipelineDescription m_pipelineDesc;
		UINT m_numRootParameters;
		CD3DX12_ROOT_PARAMETER* m_rootParameters;
		ComPtr<ID3D12RootSignature> m_rootSignature;
	};
}