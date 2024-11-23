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
			RenderQueuePass(std::move(name)),
			pShadowCBuf{ std::make_shared<ShadowCameraBuffer>(gfx, 3) }
		{
			//AddBind(pShadowCBuf);
			RegisterSink(DirectBufferBucketSink<RenderTarget>::Make("renderTarget", renderTargetVector));
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", depthStencil));
			//RegisterSink(DirectBindableSink<Bindable>::Make("shadowMap", pShadowMap));

			RegisterSource(DirectBufferBucketSource<RenderTarget>::Make("renderTarget", renderTargetVector));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", depthStencil));
		}
		void BindMainCamera(const Camera& cam) noexcept
		{
			pMainCamera = &cam;
		}
		void BindShadowCamera(const Camera& cam) noexcept
		{
			pShadowCBuf->SetCamera(&cam);
		}
		void Execute(D3D12RHI& gfx) const noexcept override
		{
			assert(pMainCamera);
			//pShadowCBuf->Update(gfx);
			pMainCamera->Update(gfx);
			RenderQueuePass::Execute(gfx);
		}
	private:
		std::shared_ptr<ShadowCameraBuffer> pShadowCBuf;
		std::shared_ptr<Bindable> pShadowMap;
		const Camera* pMainCamera = nullptr;
	};
}