#pragma once
#include "BindableCodex.h"
#include "CommonBindables.h"
#include "RootSignature.h"

namespace Renderer
{
	class PipelineState : public Bindable
	{
	public:
		PipelineState(D3D12RHI& gfx, PipelineDescription& pipelineDesc);
		~PipelineState() = default;
		void Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;

	private:
		ComPtr<ID3D12PipelineState> m_pipelineState;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDescription;
	};
}