#pragma once
#include "FullscreenPass.h"
#include "ConstantBuffer.h"

namespace Renderer
{
	class D3D12RHI;
	class PixelShader;
	class RenderTarget;

	class HorizontalBlurPass : public FullscreenPass
	{
	public:
		HorizontalBlurPass(std::string name, D3D12RHI& gfx, unsigned int fullWidth, unsigned int fullHeight);
		void Execute(D3D12RHI& gfx) const noexcept override;
	private:
		std::shared_ptr<ConstantBuffer> direction;
	};
}