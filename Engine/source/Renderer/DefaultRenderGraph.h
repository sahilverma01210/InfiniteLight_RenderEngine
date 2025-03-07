#pragma once
#include "RenderGraph.h"
#include "CameraContainer.h"

// Passes Used in this Render Graph.
#include "BufferClearPass.h"
#include "ShadowMappingPass.h"
#include "FlatPass.h"
#include "PhongPass.h"
#include "SkyboxPass.h"

namespace Renderer
{
	class DefaultRenderGraph : public RenderGraph
	{
	public:
		DefaultRenderGraph(D3D12RHI& gfx, CameraContainer& cameraContainer)
			:
			RenderGraph(gfx)
		{
			{
				auto pass = std::make_unique<BufferClearPass>(gfx, "clear");
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<ShadowMappingPass>(gfx, "shadowMap", cameraContainer);
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<FlatPass>(gfx, "flat_shading", cameraContainer);
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<PhongPass>(gfx, "phong_shading", cameraContainer);
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<SkyboxPass>(gfx, "skybox");
				AppendPass(std::move(pass));
			}

			Finalize();
		}
	};
}