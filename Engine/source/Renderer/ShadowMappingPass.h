#pragma once
#include "RenderPass.h"
#include "RenderMath.h"
#include "Camera.h"

namespace Renderer
{
	class ShadowMappingPass : public RenderPass
	{
	public:
		ShadowMappingPass(D3D12RHI& gfx, std::string name)
			:
			RenderPass(std::move(name))
		{
			m_pDepthCube = std::make_shared<DepthCubeMapTextureBuffer>(gfx, m_size);
			gfx.SetDepthBuffer(m_pDepthCube->GetBuffer());
			RegisterSource(DirectBindableSource<DepthCubeMapTextureBuffer>::Make("map", m_pDepthCube));

			SetDepthBuffer(m_pDepthCube->GetDepthBuffer(0));
		}
		void BindShadowCamera(const Camera& cam) noexcept(!IS_DEBUG)
		{
			m_pShadowCamera = &cam;
		}
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override
		{
			UINT initWidth = gfx.GetWidth();
			UINT initHeight = gfx.GetHeight();

			gfx.ResizeScreenSpace(m_size, m_size);

			for (size_t i = 0; i < 6; i++)
			{
				auto d = m_pDepthCube->GetDepthBuffer(i);
				d->Clear(gfx);
				SetDepthBuffer(std::move(d));

				m_pShadowCamera->Update(true, i);

				RenderPass::Execute(gfx);
			}
			gfx.ResizeScreenSpace(initWidth, initHeight);
		}
	private:
		void SetDepthBuffer(std::shared_ptr<DepthStencil> ds) const
		{
			const_cast<ShadowMappingPass*>(this)->m_depthStencil = std::move(ds);
		}

	private:
		static constexpr UINT m_size = 1000;
		const Camera* m_pShadowCamera = nullptr;
		std::shared_ptr<DepthCubeMapTextureBuffer> m_pDepthCube;
	};
}