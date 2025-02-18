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
				pass->SetSinkLinkage("renderTargetBuffers", "$.renderTargetBuffers");
				pass->SetSinkLinkage("depthStencilBuffer", "$.depthStencilBuffer");
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<ShadowMappingPass>(gfx, "shadowMap", cameraContainer);
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<FlatPass>(gfx, "flat_shading", cameraContainer);
				pass->SetSinkLinkage("renderTargetBuffers", "clear.renderTargetBuffers");
				pass->SetSinkLinkage("depthStencilBuffer", "clear.depthStencilBuffer");
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<PhongPass>(gfx, "phong_shading", cameraContainer);
				pass->SetSinkLinkage("shadowMap", "shadowMap.map");
				pass->SetSinkLinkage("renderTargetBuffers", "flat_shading.renderTargetBuffers");
				pass->SetSinkLinkage("depthStencilBuffer", "flat_shading.depthStencilBuffer");
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<SkyboxPass>(gfx, "skybox");
				pass->SetSinkLinkage("renderTargetBuffers", "phong_shading.renderTargetBuffers");
				pass->SetSinkLinkage("depthStencilBuffer", "phong_shading.depthStencilBuffer");
				AppendPass(std::move(pass));
			}
			SetSinkTarget("renderTargetBuffers", "skybox.renderTargetBuffers");

			Finalize();
		}
	};
}