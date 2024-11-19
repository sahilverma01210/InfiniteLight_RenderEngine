#pragma once
#include "RenderGraph.h"

namespace Renderer
{
	class D3D12RHI;

	class ScaleOutlineRenderGraph : public RenderGraph
	{
	public:
		ScaleOutlineRenderGraph(D3D12RHI& gfx);
	};
}