#pragma once
#include "RenderGraph.h"
#include "CameraContainer.h"

// Passes Used in this Render Graph.
#include "BufferClearPass.h"
#include "ShadowMappingPass.h"
#include "PhongPass.h"
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
		BlurOutlineRenderGraph(D3D12RHI& gfx, CameraContainer& cameraContainer)
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
				auto pass = std::make_unique<PhongPass>(gfx, "phong_shading", cameraContainer);
				pass->SetSinkLinkage("shadowMap", "shadowMap.map");
				pass->SetSinkLinkage("renderTargetBuffers", "clear.renderTargetBuffers");
				pass->SetSinkLinkage("depthStencilBuffer", "clear.depthStencilBuffer");
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<SkyboxPass>(gfx, "skybox");
				pass->SetSinkLinkage("renderTargetBuffers", "phong_shading.renderTargetBuffers");
				pass->SetSinkLinkage("depthStencilBuffer", "phong_shading.depthStencilBuffer");
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<OutlineMaskGenerationPass>(gfx, "outlineMask");
				pass->SetSinkLinkage("depthStencilBuffer", "skybox.depthStencilBuffer");
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<BlurOutlineDrawingPass>(gfx, "outlineDraw", gfx.GetWidth(), gfx.GetHeight());
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<HorizontalBlurPass>(gfx, "horizontal", gfx.GetWidth(), gfx.GetHeight());
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<VerticalBlurPass>(gfx, "vertical");
				pass->SetSinkLinkage("renderTargetBuffers", "skybox.renderTargetBuffers");
				pass->SetSinkLinkage("depthStencilBuffer", "outlineMask.depthStencilBuffer");
				AppendPass(std::move(pass));
			}
			{
				auto pass = std::make_unique<WireframePass>(gfx, "wireframe");
				pass->SetSinkLinkage("renderTargetBuffers", "vertical.renderTargetBuffers");
				pass->SetSinkLinkage("depthStencilBuffer", "vertical.depthStencilBuffer");
				AppendPass(std::move(pass));
			}
			SetSinkTarget("renderTargetBuffers", "wireframe.renderTargetBuffers");

			Finalize();
		}
	};
}