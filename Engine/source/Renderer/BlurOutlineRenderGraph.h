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
	public:
		BlurOutlineRenderGraph(D3D12RHI& gfx);
		void BindMainCamera(Camera& cam);
		void BindShadowCamera(Camera& cam);
	};
}