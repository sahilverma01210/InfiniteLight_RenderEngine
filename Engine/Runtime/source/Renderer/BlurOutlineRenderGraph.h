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
		void RenderWidgets(D3D12RHI& gfx);
		void SetKernelBox(int radius) noexcept;
	private:
		// private functions
		void SetKernelGauss(int radius, float sigma) noexcept;
		// private data
		enum class KernelType
		{
			Gauss,
			Box,
		} kernelType = KernelType::Gauss;
		static constexpr int maxRadius = 7;
		int radius = 4;
		float sigma = 2.0f;
		std::shared_ptr<ConstantBuffer> blurKernel;
		std::shared_ptr<ConstantBuffer> blurHorizontal;
		std::shared_ptr<ConstantBuffer> blurVertical;
	};
}