#pragma once
#include "RenderMath.h"

#include "RenderGraph.h"
#include "ConstantBuffer.h"
#include "Source.h"
#include "RenderTarget.h"
#include "DynamicConstant.h"
#include "UIManager.h"

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
	class D3D12RHI;
	class Bindable;
	class RenderTarget;
	class Camera;

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
		void DumpShadowMap(D3D12RHI& gfx, const std::string& path);
		void BindMainCamera(Camera& cam);
		void BindShadowCamera(Camera& cam);
		void SetKernelBox(int radius) noexcept(!IS_DEBUG);
	private:
		// private functions
		void SetKernelGauss(int radius, float sigma) noexcept(!IS_DEBUG);
		
	private:
		KernelType kernelType = KernelType::Gauss;
		static constexpr int maxRadius = 7;
		int radius = 4;
		float sigma = 2.0f;
		std::shared_ptr<ConstantBuffer> blurKernel;
		std::shared_ptr<ConstantBuffer> blurHorizontal;
		std::shared_ptr<ConstantBuffer> blurVertical;
	};
}