#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
#include <vector>
#include "Source.h"
#include "RenderTarget.h"
#include "Camera.h"

namespace Renderer
{
	class D3D12RHI;

	class ShadowMappingPass : public RenderQueuePass
	{
	public:
		void BindShadowCamera(const Camera& cam) noexcept
		{
			pShadowCamera = &cam;
		}
		ShadowMappingPass(D3D12RHI& gfx, std::string name)
			:
			RenderQueuePass(std::move(name))
		{
			depthStencil = std::make_unique<DepthStencil>(gfx, gfx.GetWidth(), gfx.GetHeight(), DepthUsage::ShadowDepth);
			gfx.SetDepthBuffer(depthStencil.get()->GetBuffer());
			RegisterSource(DirectBindableSource<DepthStencil>::Make("map", depthStencil));
		}
		void Execute(D3D12RHI& gfx) const noexcept override
		{
			depthStencil->Clear(gfx);
			pShadowCamera->Update(gfx);
			RenderQueuePass::Execute(gfx);
		}
		void DumpShadowMap(D3D12RHI& gfx, const std::string& path) const
		{
			//depthStencil->ToSurface(gfx).Save(path);
		}
	private:
		const Camera* pShadowCamera = nullptr;
	};
}