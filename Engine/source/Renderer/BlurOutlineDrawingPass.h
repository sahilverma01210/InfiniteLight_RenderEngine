#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
#include "Source.h"
#include "RenderTarget.h"

namespace Renderer
{
	class D3D12RHI;

	class BlurOutlineDrawingPass : public RenderQueuePass
	{
	public:
		BlurOutlineDrawingPass(D3D12RHI& gfx, std::string name, unsigned int fullWidth, unsigned int fullHeight)
			:
			RenderQueuePass(std::move(name))
		{
			renderTargetVector.push_back(std::make_unique<RenderTarget>(gfx, fullWidth, fullHeight));
			RegisterSource(DirectBindableSource<RenderTarget>::Make("scratchOut", renderTargetVector[0]));
		}
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override
		{
			renderTargetVector[0]->Clear(gfx);
			RenderQueuePass::Execute(gfx);
		}
	};
}