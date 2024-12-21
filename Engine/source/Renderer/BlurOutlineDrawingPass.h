#pragma once
#include "RenderQueuePass.h"

namespace Renderer
{
	class BlurOutlineDrawingPass : public RenderQueuePass
	{
	public:
		BlurOutlineDrawingPass(D3D12RHI& gfx, std::string name, unsigned int fullWidth, unsigned int fullHeight)
			:
			RenderQueuePass(std::move(name))
		{
			m_renderTargetVector.push_back(std::make_unique<RenderTarget>(gfx, fullWidth, fullHeight));
		}
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override
		{
			m_renderTargetVector[0]->Clear(gfx);
			RenderQueuePass::Execute(gfx);
		}
	};
}