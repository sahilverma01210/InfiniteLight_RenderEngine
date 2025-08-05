#pragma once
#include "GraphicsResource.h"

namespace Renderer
{
	class PipelineState : public GraphicsResource
	{
	public:
		PipelineState(D3D12RHI& gfx, PipelineDescription& pipelineDesc);
		~PipelineState() = default;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyType() { return m_topologyType; }
		void Bind() noexcept(!IS_DEBUG);

	private:
		D3D12RHI& m_gfx;
		ComPtr<ID3D12PipelineState> m_pipelineState;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_graphicsPsoDescription;
		D3D12_COMPUTE_PIPELINE_STATE_DESC m_computePsoDescription;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	};
}