#pragma once
#include "RenderQueuePass.h"

namespace Renderer
{
	class HorizontalBlurPass : public RenderQueuePass
	{
	public:
		HorizontalBlurPass(D3D12RHI& gfx, std::string name, unsigned int fullWidth, unsigned int fullHeight)
			:
			RenderQueuePass(std::move(name))
		{
			m_renderTargetVector.push_back(std::make_shared<RenderTarget>(gfx, fullWidth, fullHeight));
		}

		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override
		{
			ID3D12Resource* blurTargetBuffer = gfx.GetRenderTargetBuffers().back().Get();
			gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			RenderQueuePass::Execute(gfx);

			gfx.TransitionResource(blurTargetBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
	};
}