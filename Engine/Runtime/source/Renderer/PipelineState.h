#pragma once
#include "Bindable.h"
#include "RootSignature.h"

namespace Renderer
{
	struct PipelineDescription
	{
		ID3DBlob& vertexShader;
		ID3DBlob& pixelShader;
		D3D12_INPUT_ELEMENT_DESC& inputElementDescs;
		UINT numElements;
		UINT numConstants;
		UINT numConstantBufferViews;
	};

	class PipelineState : public Bindable
	{
	public:
		PipelineState(D3D12RHI& gfx, PipelineDescription& pipelineDesc);
		void Bind(D3D12RHI& gfx) noexcept override;
	private:
		ComPtr<ID3D12PipelineState> m_pipelineState;
		std::unique_ptr<RootSignature> m_rootSignatureObject;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDescription;
	};
}