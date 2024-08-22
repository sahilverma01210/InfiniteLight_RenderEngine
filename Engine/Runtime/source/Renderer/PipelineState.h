#pragma once
#include "Bindable.h"

namespace Renderer
{
	class PipelineState : public Bindable
	{
	public:
		PipelineState(D3D12RHI& gfx, ID3DBlob& vertexShader, ID3DBlob& pixelShader, D3D12_INPUT_ELEMENT_DESC& inputElementDescs, UINT numElements);
		void Bind(D3D12RHI& gfx) noexcept override;
	private:
		ComPtr<ID3D12PipelineState> m_pipelineState;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesccription;
	};
}