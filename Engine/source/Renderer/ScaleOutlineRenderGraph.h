#pragma once
#include "RenderGraph.h"

// Passes Used in this Render Graph.
#include "BufferClearPass.h"
#include "LambertianPass.h"
#include "OutlineDrawingPass.h"
#include "OutlineMaskGenerationPass.h"

namespace Renderer
{
	class D3D12RHI;

	class ScaleOutlineRenderGraph : public RenderGraph
	{
	public:
		ScaleOutlineRenderGraph(D3D12RHI& gfx);
	};
}