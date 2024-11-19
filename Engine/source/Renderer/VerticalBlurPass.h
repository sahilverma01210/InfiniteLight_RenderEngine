#pragma once
#include "FullscreenPass.h"
#include "ConstantBuffer.h"

namespace Renderer
{
	class D3D12RHI;
	class PixelShader;
	class RenderTarget;

	class VerticalBlurPass : public FullscreenPass
	{
	public:
		VerticalBlurPass(std::string name, D3D12RHI& gfx);
		void Execute(D3D12RHI& gfx) const noexcept override;
	private:
		std::shared_ptr<RenderTarget> blurTarget;
		std::shared_ptr<ConstantBuffer> direction;
	};
}