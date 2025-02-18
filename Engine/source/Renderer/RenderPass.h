#pragma once
#include "Pass.h"
#include "Job.h"

namespace Renderer
{
	class RenderPass : public Pass
	{
	public:
		RenderPass(std::string name);
		void Accept(Job job) noexcept(!IS_DEBUG);
		void Finalize() override;
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;
		void Reset() noexcept(!IS_DEBUG) override;
	private:
		void BindRenderGraphResources(D3D12RHI& gfx) const noexcept(!IS_DEBUG);

	protected:
		bool m_depthOnlyPass = false;
		D3D12_PRIMITIVE_TOPOLOGY m_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		std::unique_ptr<RootSignature> m_rootSignature;
		std::unique_ptr<PipelineState> m_pipelineStateObject;
		std::shared_ptr<RenderTarget> m_renderTarget;
		std::shared_ptr<DepthStencil> m_depthStencil;
	private:
		std::vector<Job> m_jobs;
	};
}