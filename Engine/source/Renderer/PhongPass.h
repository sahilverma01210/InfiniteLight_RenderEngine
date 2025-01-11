#pragma once
#include "RenderPass.h"
#include "CameraContainer.h"

namespace Renderer
{
	class PhongPass : public RenderPass
	{
	public:
		PhongPass(D3D12RHI& gfx, std::string name, CameraContainer& cameraContainer)
			:
			RenderPass(std::move(name)),
			m_cameraContainer(cameraContainer)
		{
			RegisterSink(DirectBufferBucketSink<RenderTarget>::Make("renderTargetBuffers", m_renderTargetVector));
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencilBuffer", m_depthStencil));
			RegisterSink(DirectBindableSink<DepthCubeMapTextureBuffer>::Make("shadowMap", m_pShadowMap));

			RegisterSource(DirectBufferBucketSource<RenderTarget>::Make("renderTargetBuffers", m_renderTargetVector));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencilBuffer", m_depthStencil));
		}
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override
		{
			m_cameraContainer.GetActiveCamera().Update();

			gfx.TransitionResource(m_pShadowMap->GetBuffer(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			RenderPass::Execute(gfx);
			gfx.TransitionResource(m_pShadowMap->GetBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}

	private:
		CameraContainer& m_cameraContainer;
		std::shared_ptr<DepthCubeMapTextureBuffer> m_pShadowMap;
	};
}