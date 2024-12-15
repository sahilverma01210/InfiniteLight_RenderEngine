#pragma once
#include "RenderQueuePass.h"
#include "RenderMath.h"
#include "Camera.h"

namespace Renderer
{
	class ShadowMappingPass : public RenderQueuePass
	{
	public:
		ShadowMappingPass(D3D12RHI& gfx, std::string name)
			:
			RenderQueuePass(std::move(name))
		{
			m_pDepthCube = std::make_shared<DepthCubeMapTextureBuffer>(gfx, m_size);
			gfx.SetDepthBuffer(m_pDepthCube->GetBuffer());
			RegisterSource(DirectBindableSource<DepthCubeMapTextureBuffer>::Make("map", m_pDepthCube));

			XMStoreFloat4x4(
				&m_projection,
				XMMatrixPerspectiveFovLH(PI / 2.0f, 1.0f, 0.5f, 100.0f)
			);
			// +x
			XMStoreFloat3(&m_cameraDirections[0], XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
			XMStoreFloat3(&m_cameraUps[0], XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			// -x
			XMStoreFloat3(&m_cameraDirections[1], XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f));
			XMStoreFloat3(&m_cameraUps[1], XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			// +y
			XMStoreFloat3(&m_cameraDirections[2], XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			XMStoreFloat3(&m_cameraUps[2], XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
			// -y
			XMStoreFloat3(&m_cameraDirections[3], XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f));
			XMStoreFloat3(&m_cameraUps[3], XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
			// +z
			XMStoreFloat3(&m_cameraDirections[4], XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
			XMStoreFloat3(&m_cameraUps[4], XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			// -z
			XMStoreFloat3(&m_cameraDirections[5], XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
			XMStoreFloat3(&m_cameraUps[5], XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			SetDepthBuffer(m_pDepthCube->GetDepthBuffer(0));
		}
		void BindShadowCamera(const Camera& cam) noexcept(!IS_DEBUG)
		{
			m_pShadowCamera = &cam;
		}
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override
		{
			const auto shadowPos = m_pShadowCamera->GetPos();
			const auto pos = XMLoadFloat3(&shadowPos);

			UINT initWidth = gfx.GetWidth();
			UINT initHeight = gfx.GetHeight();

			gfx.ResizeScreenSpace(m_size, m_size);
			gfx.SetProjection(XMLoadFloat4x4(&m_projection));

			for (size_t i = 0; i < 6; i++)
			{
				auto d = m_pDepthCube->GetDepthBuffer(i);
				d->Clear(gfx);
				SetDepthBuffer(std::move(d));
				const auto lookAt = pos + XMLoadFloat3(&m_cameraDirections[i]);
				gfx.SetCamera(XMMatrixLookAtLH(pos, lookAt, XMLoadFloat3(&m_cameraUps[i])));
				RenderQueuePass::Execute(gfx);
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
		XMFLOAT4X4 m_projection;
		std::vector<XMFLOAT3> m_cameraDirections{ 6 };
		std::vector<XMFLOAT3> m_cameraUps{ 6 };
	};
}