#pragma once
#include "BindingPass.h"
#include "Plane.h"

namespace Renderer
{
	class FullscreenPass : public BindingPass
	{
	public:
		FullscreenPass(const std::string name, D3D12RHI& gfx) noexcept(!IS_DEBUG);
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override;

	protected:
		PipelineDescription m_pipelineDesc{};
		std::shared_ptr<VertexBuffer> m_vertexBindable;
		std::shared_ptr<IndexBuffer> m_indexBindable;
		std::unique_ptr<RootSignature> m_rootSignBindable;
		std::unique_ptr<PipelineState> m_psoBindable;
		std::unique_ptr<ShaderResourceView> m_srvBindable;
	};
}