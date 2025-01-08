#pragma once
#include "RenderGraph.h"
#include "CameraContainer.h"

// Passes Used in this Render Graph.
#include "BufferClearPass.h"
#include "ShadowMappingPass.h"
#include "LambertianPass.h"
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
				auto pass = std::make_unique<BufferClearPass>("clear");
				pass->SetSinkLinkage("renderTargetBuffers", "$.renderTargetBuffers");
				pass->SetSinkLinkage("depthStencilBuffer", "$.depthStencilBuffer");
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<ShadowMappingPass>(gfx, "shadowMap", cameraContainer);
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<LambertianPass>(gfx, "lambertian", cameraContainer);
				pass->SetSinkLinkage("shadowMap", "shadowMap.map");
				pass->SetSinkLinkage("renderTargetBuffers", "clear.renderTargetBuffers");
				pass->SetSinkLinkage("depthStencilBuffer", "clear.depthStencilBuffer");
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<SkyboxPass>(gfx, "skybox");
				pass->SetSinkLinkage("renderTargetBuffers", "lambertian.renderTargetBuffers");
				pass->SetSinkLinkage("depthStencilBuffer", "lambertian.depthStencilBuffer");
				AppendPass(std::move(pass));
			}
			SetSinkTarget("renderTargetBuffers", "skybox.renderTargetBuffers");

			Finalize();
		}
	};
}