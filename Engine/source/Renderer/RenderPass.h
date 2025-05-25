#pragma once
#include "../Common/ILMath.h"

#include "Pass.h"
#include "Drawable.h"

namespace Renderer
{
	enum RenderPassType
	{
		Graphics,
		Compute
	};

	class RenderPass : public Pass
	{
	public:
		RenderPass(std::string name, RenderPassType type = RenderPassType::Graphics);
		void Accept(const Drawable& drawable) noexcept(!IS_DEBUG);
		void Finalize() override;
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;
		void Reset() noexcept(!IS_DEBUG) override;

	public:
		static inline ResourceHandle m_cameraDataHandle = 0;
	protected:
		RenderPassType m_renderPassType;
		std::unique_ptr<RootSignature> m_rootSignature;
		std::unique_ptr<PipelineState> m_pipelineStateObject;
		std::vector<std::shared_ptr<D3D12Resource>> m_renderTargets;
		std::shared_ptr<DepthStencil> m_depthStencil;
	private:
		std::list<std::reference_wrapper<const Drawable>> m_drawables;
	};
}