#pragma once
#include "RenderQueuePass.h"
#include "Camera.h"

namespace Renderer
{
	class LambertianPass : public RenderQueuePass
	{
	public:
		LambertianPass(D3D12RHI& gfx, std::string name)
			:
			RenderQueuePass(std::move(name))
		{
			RegisterSink(DirectBufferBucketSink<RenderTarget>::Make("renderTarget", m_renderTargetVector));
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", m_depthStencil));
			RegisterSink(DirectBindableSink<DepthCubeMapTextureBuffer>::Make("shadowMap", m_pShadowMap));

			RegisterSource(DirectBufferBucketSource<RenderTarget>::Make("renderTarget", m_renderTargetVector));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", m_depthStencil));
		}
		void BindMainCamera(const Camera& cam) noexcept(!IS_DEBUG)
		{
			m_pMainCamera = &cam;
		}
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override
		{
			assert(m_pMainCamera);
			m_pMainCamera->Update(gfx);
			gfx.TransitionResource(m_pShadowMap->GetBuffer(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			RenderQueuePass::Execute(gfx);
			gfx.TransitionResource(m_pShadowMap->GetBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}

	private:
		std::shared_ptr<DepthCubeMapTextureBuffer> m_pShadowMap;
		const Camera* m_pMainCamera = nullptr;
	};
}