#pragma once
#include "RenderGraph.h"

// Passes Used in this Render Graph.
#include "BufferClearPass.h"
#include "ShadowMappingPass.h"
#include "LambertianPass.h"
#include "SkyboxPass.h"
#include "OutlineMaskGenerationPass.h"
#include "BlurOutlineDrawingPass.h"
#include "HorizontalBlurPass.h"
#include "VerticalBlurPass.h"
#include "WireframePass.h"

namespace Renderer
{
	class BlurOutlineRenderGraph : public RenderGraph
	{
	private:
		enum class KernelType
		{
			Gauss,
			Box,
		};

	public:
		BlurOutlineRenderGraph(D3D12RHI& gfx);
		void RenderWidgets(D3D12RHI& gfx);
		void BindMainCamera(Camera& cam);
		void BindShadowCamera(Camera& cam);
		void SetKernelBox(int radius) noexcept(!IS_DEBUG);
	private:
		// private functions
		void SetKernelGauss(int radius, float sigma) noexcept(!IS_DEBUG);
		
	private:
		KernelType m_kernelType = KernelType::Gauss;
		static constexpr int m_maxRadius = 7;
		int m_radius = 4;
		float m_sigma = 2.0f;
		std::shared_ptr<ConstantBuffer> m_blurKernel;
		std::shared_ptr<ConstantBuffer> m_blurHorizontal;
		std::shared_ptr<ConstantBuffer> m_blurVertical;
	};
}