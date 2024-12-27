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
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override;
		void Reset() noexcept(!IS_DEBUG) override;
	private:
		void BindBufferResources(D3D12RHI& gfx) const noexcept(!IS_DEBUG);

	protected:
		std::vector<std::shared_ptr<RenderTarget>> m_renderTargetVector;
		std::shared_ptr<DepthStencil> m_depthStencil;
	private:
		std::vector<Job> m_jobs;
	};
}