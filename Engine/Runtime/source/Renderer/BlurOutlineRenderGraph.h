#pragma once
#include "RenderGraph.h"
#include "ConstantBuffer.h"

namespace Renderer
{
	class D3D12RHI;
	class Bindable;
	class RenderTarget;

	class BlurOutlineRenderGraph : public RenderGraph
	{
	public:
		BlurOutlineRenderGraph(D3D12RHI& gfx);
	private:
		// private functions
		void SetKernelGauss(int radius, float sigma) noexcept;
		// private data
		static constexpr int maxRadius = 7;
		static constexpr int radius = 4;
		static constexpr float sigma = 2.0f;
		std::shared_ptr<ConstantBuffer> blurKernel;
		std::shared_ptr<ConstantBuffer> blurDirection;
	};
}