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
		PipelineDescription pipelineDesc{};
		std::shared_ptr<VertexBuffer> vertexBindable;
		std::shared_ptr<IndexBuffer> indexBindable;
		std::unique_ptr<RootSignature> rootSignBindable;
		std::unique_ptr<PipelineState> psoBindable;
		std::unique_ptr<ShaderResourceView> srvBindable;
	};
}