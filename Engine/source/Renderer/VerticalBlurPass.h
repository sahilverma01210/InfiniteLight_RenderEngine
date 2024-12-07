#pragma once
#include "FullscreenPass.h"
#include "ConstantBuffer.h"
#include "Sink.h"
#include "Source.h"

namespace Renderer
{
	class VerticalBlurPass : public FullscreenPass
	{
	public:
		VerticalBlurPass(std::string name, D3D12RHI& gfx);
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override;

	private:
		std::shared_ptr<RenderTarget> blurTarget;
		std::shared_ptr<ConstantBuffer> direction;
	};
}