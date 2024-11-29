#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
#include "Sink.h"
#include "Source.h"
#include "Camera.h"
#include "DepthStencil.h"
#include "ShadowCameraBuffer.h"

namespace Renderer
{
	class D3D12RHI;

	class LambertianPass : public RenderQueuePass
	{
	public:
		LambertianPass(D3D12RHI& gfx, std::string name)
			:
			RenderQueuePass(std::move(name))/*,
			pShadowCBuf{ std::make_shared<ShadowCameraBuffer>(gfx, 1) }*/
		{
			//AddBind(pShadowCBuf);
			RegisterSink(DirectBufferBucketSink<RenderTarget>::Make("renderTarget", renderTargetVector));
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", depthStencil));
			RegisterSink(DirectBindableSink<DepthStencil>::Make("shadowMap", pShadowMap));

			RegisterSource(DirectBufferBucketSource<RenderTarget>::Make("renderTarget", renderTargetVector));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", depthStencil));
		}
		void BindMainCamera(const Camera& cam) noexcept
		{
			pMainCamera = &cam;
		}
		void BindShadowCamera(const Camera& cam) noexcept
		{
			//pShadowCBuf->SetCamera(&cam);
		}
		void Execute(D3D12RHI& gfx) const noexcept override
		{
			assert(pMainCamera);
			//pShadowCBuf->Update(gfx);
			pMainCamera->Update(gfx);
			gfx.TransitionResource(pShadowMap->GetBuffer(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			RenderQueuePass::Execute(gfx);
			gfx.TransitionResource(pShadowMap->GetBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}
	private:
		//std::shared_ptr<ShadowCameraBuffer> pShadowCBuf;
		std::shared_ptr<DepthStencil> pShadowMap;
		const Camera* pMainCamera = nullptr;
	};
}