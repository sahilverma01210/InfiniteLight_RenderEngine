#pragma once
#include "FullscreenPass.h"
#include "ConstantBuffer.h"
#include "RenderTarget.h"
#include "Sink.h"
#include "Source.h"

namespace Renderer
{
	class HorizontalBlurPass : public FullscreenPass
	{
	public:
		HorizontalBlurPass(std::string name, D3D12RHI& gfx, unsigned int fullWidth, unsigned int fullHeight);
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override;

	private:
		std::shared_ptr<RenderTarget> blurTarget;
		std::shared_ptr<ConstantBuffer> direction;
	};
}