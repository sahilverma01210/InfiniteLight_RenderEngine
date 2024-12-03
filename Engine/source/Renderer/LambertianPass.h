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
			RenderQueuePass(std::move(name))
		{
			RegisterSink(DirectBufferBucketSink<RenderTarget>::Make("renderTarget", renderTargetVector));
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", depthStencil));
			RegisterSink(DirectBindableSink<DepthCubeMapTextureBuffer>::Make("shadowMap", pShadowMap));

			RegisterSource(DirectBufferBucketSource<RenderTarget>::Make("renderTarget", renderTargetVector));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", depthStencil));
		}
		void BindMainCamera(const Camera& cam) noexcept
		{
			pMainCamera = &cam;
		}
		void BindShadowCamera(const Camera& cam) noexcept
		{
		}
		void Execute(D3D12RHI& gfx) const noexcept override
		{
			assert(pMainCamera);
			pMainCamera->Update(gfx);
			gfx.TransitionResource(pShadowMap->GetBuffer(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			RenderQueuePass::Execute(gfx);
			gfx.TransitionResource(pShadowMap->GetBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}
	private:
		std::shared_ptr<DepthCubeMapTextureBuffer> pShadowMap;
		const Camera* pMainCamera = nullptr;
	};
}